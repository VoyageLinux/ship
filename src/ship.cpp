#include "ship_env.h"

//std::string ship_lib_path = "/var/lib/ship/";
std::string ship_lib_path = "/home/ship/";

boost::property_tree::ptree pt;

std::ostream& operator<<(std::ostream& os, const ShipMode& mode) {
    switch (mode) {
        case ShipMode::VM:
            return os << "VM";
        case ShipMode::CONTAINER:
            return os << "Container";
        default:
            return os << "Unknown";
    }
}

std::ostream& operator<<(std::ostream& os, const ShipAction& action) {
   switch (action) {
        case ShipAction::CREATE:
            return os << "Create";
        case ShipAction::DELETE:
            return os << "Delete";
        case ShipAction::SAVE:
            return os << "Save";
        case ShipAction::VIEW:
            return os << "View";
        case ShipAction::UPGRADE:
            return os << "Upgrade";
        case ShipAction::LIST:
            return os << "List";
        case ShipAction::START:
            return os << "Start";
        case ShipAction::RESTART:
            return os << "Restart";
        case ShipAction::PAUSE:
            return os << "Pause";
        case ShipAction::STOP:
            return os << "Stop";
        case ShipAction::RESUME:
            return os << "Resume";
        case ShipAction::EXEC:
            return os << "Exec";
        case ShipAction::PACKAGE_DOWNLOAD:
            return os << "Package Download";
        case ShipAction::PACKAGE_SEARCH:
            return os << "Package Search";
        case ShipAction::PACKAGE_REMOVE:
            return os << "Package Remove";
        case ShipAction::RECEIVE:
            return os << "Receive";
        case ShipAction::SEND:
            return os << "Send";
        case ShipAction::SHUTDOWN:
            return os << "Shutdown";
        case ShipAction::OPTIMIZE:
            return os << "Optimize";
        case ShipAction::COMPRESS:
            return os << "Compress";
        default:
            return os << "Unknown";
   }
}

std::unordered_map<std::string, PackageManagerCommands> package_managers = {
    {"ship", {"sudo ship", "sudo ship search", "sudo ship install", "sudo ship remove"}},
    {"apk", {"sudo apk", "sudo apk search", "sudo apk add", "sudo apk del"}},
    {"apt-get", {"sudo apt-get", "sudo apt-cache search", "sudo apt-get install", "sudo apt-get remove"}},
    {"dnf", {"sudo dnf", "sudo dnf search", "sudo dnf install", "sudo dnf remove"}},
    {"zypper", {"sudo zypper", "sudo zypper search", "sudo zypper install", "sudo zypper remove"}},
    {"emerge", {"sudo emerge", "sudo emerge --search", "sudo emerge", "sudo emerge --unmerge"}},
    {"pacman", {"sudo pacman", "sudo pacman -Ss", "sudo pacman -S", "sudo pacman -R"}},
    {"yum", {"sudo yum", "sudo yum search", "sudo yum install", "sudo yum remove"}},
    {"apx", {"sudo apx", "sudo apx search", "sudo apx install", "sudo apx remove"}},
    {"pkgtool", {"sudo pkgtool", "sudo pkgtool search", "sudo pkgtool install", "sudo pkgtool remove"}},
    {"xbps", {"sudo xbps", "sudo xbps-query -Rs", "sudo xbps-install", "sudo xbps-remove"}},
    {"paludis", {"sudo cave", "sudo cave search", "sudo cave resolve -x", "sudo cave uninstall"}},
    {"urpmi", {"sudo urpmi", "sudo urpmq", "sudo urpmi", "sudo urpme"}},
    {"tce-load", {"sudo tce-load", "sudo tce-ab search", "sudo tce-load -wi", "sudo tce-remove"}},
    {"equo", {"sudo equo", "sudo equo search", "sudo equo install", "sudo equo remove"}},
    {"tazpkg", {"sudo tazpkg", "sudo tazpkg search", "sudo tazpkg install", "sudo tazpkg remove"}}
};

std::ostream& operator<<(std::ostream& os, const TestedVM& vm) {
    switch (vm) {
        case TestedVM::tails:
            return os << "tails";
        case TestedVM::whonix:
            return os << "whonix";
        case TestedVM::debian:
            return os << "debian";
        case TestedVM::ubuntu:
            return os << "ubuntu";
        case TestedVM::arch:
            return os << "arch";
        case TestedVM::gentoo:
            return os << "gentoo";
        case TestedVM::fedora:
            return os << "fedora";
        case TestedVM::alpine:
            return os << "alpine";
        case TestedVM::centos:
            return os << "centos";
        case TestedVM::freebsd:
            return os << "freebsd";
        case TestedVM::openbsd:
            return os << "openbsd";
        case TestedVM::netbsd:
            return os << "netbsd";
        case TestedVM::dragonflybsd:
            return os << "dragonflybsd";
        case TestedVM::windows:
            return os << "windows";
        default:
            return os << "";
    }
}

// Initializing the struct with empty strings
ShipEnviornment ship_env = {
    ShipMode::UNKNOWN,  // mode
    ShipAction::UNKNOWN,  // action
    "",  // command
    "",  // name
    "",  // package_manager_name
    "",  // packages
    TestedVM::UNKNOWN, // os 
    "",  // source
    "",  // source_local
    "",  // memory_limit
    "",  // cpu_limit
    "",  // image
    "",  // iso_path
    "",  // disk_image_path
};

void process_operands(int argc, char *argv[]) {
    namespace po = boost::program_options;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Show help message")
        ("container,ctr", "Set mode to CONTAINER")
        ("virtual-machine,vm", "Set mode to VM")
        ("name", po::value<std::string>(), "Set name")
        ("source", po::value<std::string>(), "Set source")
        ("source-local", po::value<std::string>(), "Set local source")
        ("image", po::value<std::string>(), "Set image")
        ("cpus", po::value<std::string>(), "Set CPU limit")
        ("memory,mem", po::value<std::string>(), "Set memory limit")
        ("command", po::value<std::string>(), "Set command")
        ("packages", po::value<std::vector<std::string>>()->multitoken(), "Set packages")
        ("aur", "Set package manager to pacman")
        ("apk", "Set package manager to apk")
        ("dnf", "Set package manager to dnf")
        ("apt", "Set package manager to apt")
        ("nix", "Set package manager to nix")
        ("parameters,p", po::value<std::string>(), "Set additional parameters");

    po::positional_options_description positional;
    positional.add("action", 1);
    positional.add("name", 1);

    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("action", po::value<std::string>(), "Action");

    po::options_description all;
    all.add(desc).add(hidden);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(all).positional(positional).run(), vm);
        po::notify(vm);
    } catch (const po::error &ex) {
        std::cout << ex.what() << "\n";
        return;
    }

    if (vm.count("help")) {
        show_help();
        exit(0);
    }

    if (vm.count("container")) {
        ship_env.mode = ShipMode::CONTAINER;
    } else if (vm.count("virtual-machine")) {
        ship_env.mode = ShipMode::VM;
    }

    if (vm.count("action")) {
        std::string action = vm["action"].as<std::string>();
        if (action == "create") {
            ship_env.action = ShipAction::CREATE;
        } else if (action == "start") {
            ship_env.action = ShipAction::START;
        } else if (action == "restart" || action == "reboot") {
            ship_env.action = ShipAction::RESTART;
        } else if (action == "delete") {
            ship_env.action = ShipAction::DELETE;
        } else if (action == "list") {
            ship_env.action = ShipAction::LIST;
        } else if (action == "view" || action == "enter") {
            ship_env.action = ShipAction::VIEW;
        } else if (action == "save") {
            ship_env.action = ShipAction::SAVE;
        } else if (action == "shutdown") {
            ship_env.action = ShipAction::SHUTDOWN;
        } else if (action == "stop") {
            ship_env.action = ShipAction::STOP;
        } else if (action == "pause" || action == "suspend") {
            ship_env.action = ShipAction::PAUSE;
        } else if (action == "resume") {
            ship_env.action = ShipAction::RESUME;
        } else if (action == "upgrade") {
            ship_env.action = ShipAction::UPGRADE;
        } else if (action == "exec" || action == "run") {
            ship_env.action = ShipAction::EXEC;
        } else if (action == "package_download" || action == "download_packages") {
            ship_env.action = ShipAction::PACKAGE_DOWNLOAD;
        } else if (action == "package_remove" || action == "remove_packages") {
            ship_env.action = ShipAction::PACKAGE_REMOVE;
        } else if (action == "package_search" || action == "search_packages") {
            ship_env.action = ShipAction::PACKAGE_SEARCH;
        } else if (action == "receive") {
            ship_env.action = ShipAction::RECEIVE;
        } else if (action == "send") {
            ship_env.action = ShipAction::SEND;
        } else if (action == "optimize") {
            ship_env.action = ShipAction::OPTIMIZE;
        } else if (action == "compress") {
            ship_env.action = ShipAction::COMPRESS;
        } else {
            std::cout << "Ship found unknown operand " << action << " for entity " << ship_env.mode << "\n";
            std::cout << "For more information try ship --help\n";
            exit(1);
        }
    }

    if (vm.count("name")) {
        ship_env.name = vm["name"].as<std::string>();
    }

    if (vm.count("source")) {
        ship_env.source = vm["source"].as<std::string>();
        ship_env.source_local = "";
    }

    if (vm.count("source-local")) {
        ship_env.source_local = vm["source-local"].as<std::string>();
        ship_env.source = "";
    }

    if (vm.count("image")) {
        ship_env.image = vm["image"].as<std::string>();
    }

    if (vm.count("cpus")) {
        ship_env.cpu_limit = vm["cpus"].as<std::string>();
    }

    if (vm.count("memory")) {
        ship_env.memory_limit = vm["memory"].as<std::string>();
    }

    if (vm.count("command")) {
        ship_env.command = vm["command"].as<std::string>();
    }

    if (vm.count("parameters")) {
        ship_env.command += " " + vm["parameters"].as<std::string>();
    }

    if (vm.count("packages")) {
        auto packages = vm["packages"].as<std::vector<std::string>>();
        for (const auto &pkg : packages) {
            ship_env.packages += pkg + " ";
        }
    }

    if (vm.count("aur")) {
        ship_env.package_manager_name = "pacman";
    }

    if (vm.count("apk")) {
        ship_env.package_manager_name = "apk";
    }

    if (vm.count("dnf")) {
        ship_env.package_manager_name = "dnf";
    }

    if (vm.count("apt")) {
        ship_env.package_manager_name = "apt";
    }

    if (vm.count("nix")) {
        ship_env.package_manager_name = "nix";
    }
}

void exec_action() {
    if (ship_env.mode == ShipMode::VM) {
        exec_action_for_vm();
    } else if (ship_env.mode == ShipMode::CONTAINER) {
        exec_action_for_container();
    } else {
        std::cout << "Are you using this action for vms " << ship_env.name << "? (y/n,Note:The default behaviour is that the action is assumed to be for containers): ";
        std::string confirm;
        getline(std::cin, confirm);

        if (confirm != "y" && confirm != "Y") {
            ship_env.mode = ShipMode::CONTAINER;
            exec_action_for_container();
        } else {
            ship_env.mode = ShipMode::VM;
            exec_action_for_vm();
        }

    }
}
