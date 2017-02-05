#include <libudev.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <memory>

struct UdevDeleter {
    void operator()(udev *dev) const { udev_unref(dev); }
};

struct UdevEnumerateDeleter {
    void operator()(udev_enumerate *enumerate) const {
        udev_enumerate_unref(enumerate);
    }
};

struct UdevDeviceDeleter {
    void operator()(udev_device *device) const { udev_device_unref(device); }
};

struct UdevMonitorDeleter {
    void operator()(udev_monitor *monitor) const {
        udev_monitor_unref(monitor);
    }
};

using UdevPtr = std::unique_ptr<udev, UdevDeleter>;
using UdevEnumeratePtr = std::unique_ptr<udev_enumerate, UdevEnumerateDeleter>;
using UdevDevicePtr = std::unique_ptr<udev_device, UdevDeviceDeleter>;
using UdevMonitorPtr = std::unique_ptr<udev_monitor, UdevMonitorDeleter>;

bool checkIfDeviceAvailable(udev *udev, const char *subsystem,
                            const char *vendor, const char *product) {
    UdevEnumeratePtr enumerate(udev_enumerate_new(udev));
    udev_enumerate_add_match_subsystem(enumerate.get(), subsystem);
    udev_enumerate_scan_devices(enumerate.get());
    udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate.get());
    udev_list_entry *dev_list_entry;

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        path = udev_list_entry_get_name(dev_list_entry);
        udev_device *dev = udev_device_new_from_syspath(udev, path);
        UdevDevicePtr usbDev(udev_device_get_parent_with_subsystem_devtype(
            dev, "usb", "usb_device"));
        if (strcmp(udev_device_get_sysattr_value(usbDev.get(), "idVendor"),
                   vendor) == 0 &&
            strcmp(udev_device_get_sysattr_value(usbDev.get(), "idProduct"),
                   product) == 0) {
            return true;
        }
    }

    return false;
}

bool waitOnMonitor(int fd, udev_monitor *mon) {
    fd_set fds;
    struct timeval tv;
    int ret;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    ret = select(fd + 1, &fds, NULL, NULL, &tv);

    if (ret > 0 && FD_ISSET(fd, &fds)) {
        UdevDevicePtr dev(udev_monitor_receive_device(mon));
    }

    return ret >= 0;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0]
                  << " <SUBSYSTEM> <VENDOR> <PRODUCT>\n";
        return EXIT_FAILURE;
    }

    UdevPtr udev(udev_new());

    if (!udev) {
        std::cout << "Failed to execute udev_new()";
        return EXIT_FAILURE;
    }

    UdevMonitorPtr mon(udev_monitor_new_from_netlink(udev.get(), "udev"));
    udev_monitor_filter_add_match_subsystem_devtype(mon.get(), argv[1], NULL);
    udev_monitor_enable_receiving(mon.get());
    int fd = udev_monitor_get_fd(mon.get());

    if (checkIfDeviceAvailable(udev.get(), argv[1], argv[2], argv[3])) {
        return EXIT_SUCCESS;
    }

    while (waitOnMonitor(fd, mon.get())) {
        if (checkIfDeviceAvailable(udev.get(), argv[1], argv[2], argv[3])) {
            return EXIT_SUCCESS;
        }
    }

    return EXIT_FAILURE;
}
