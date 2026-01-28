#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h>   // ONLY after winsock2

#include "Controller.h"
#include <filesystem>
#include <iostream>
#include <thread>
#include "Logger.h"
#include "UiPipe.h"

Controller::Controller::Controller(ProcessMonitor& procm, HostsBlocker& hostB, ConfigManager& configm, BackendClient& backend)
    : processManager(procm), hostsBlocker(hostB), configManager(configm), backendClient(backend) {
    configPath = "C:\\FamilyBlockService\\config.json";
    std::filesystem::create_directories("C:\\FamilyBlockService");

}

void Controller::requestStop() {
    stopRequested.store(true);
}

void Controller::runService() {
    
    try {
        

        cfg = configManager.load(configPath);
        
        if (cfg.isRunning) {
            log("enterAllowedMode");
            enterAllowedMode();
        }
        else {
            log("enterBlockedMode");
            enterBlockedMode();
        }

        while (!stopRequested.load()) {
            secondsSinceLastReload++;

           

            if (secondsSinceLastReload >= 60) {
                auto updated = backendClient.heartbeat(cfg);
                if (updated) {
                    applyBackendConfig(*updated);
                }
                secondsSinceLastReload = 0;
            }
            
            if (cfg.isRunning) {
                std::string msg = "You have " + std::to_string(cfg.remainingMinutes) + " minutes left";
                SendUiMessage(msg.c_str());

                log("tickAllowedMode");
                tickAllowedMode();
            }
            else {
                log("tickBlockedMode");
                tickBlockedMode();
            }

            for (int i = 0; i < 10 && !stopRequested.load(); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
 
    }
    catch (const std::exception& ex) {
        
        std::cerr << "[Controller] Fatal error: " << ex.what() << std::endl;
    }
    catch (...) {
        std::cerr << "[Controller] Unknown fatal error" << std::endl;
    }
    
    // ABSOLUTE SAFETY: always restore hosts file
    try {
        hostsBlocker.removeBlock();
    }
    catch (const std::exception& ex) {
        std::cerr << "[Controller] Failed to remove hosts block on exit: "
            << ex.what() << std::endl;
    }
    std::cout << "[Controller] runService() exiting" << std::endl;
}

void Controller::applyBackendConfig(const TimeConfig& backendCfg) {
    bool wasRunning = cfg.isRunning;

    cfg.isRunning = backendCfg.isRunning;
    cfg.remainingMinutes = backendCfg.remainingMinutes;
    cfg.blockedApps = backendCfg.blockedApps;
    cfg.blockedBrowserApps = backendCfg.blockedBrowserApps;
    cfg.blockedWebsites = backendCfg.blockedWebsites;

    if (wasRunning != cfg.isRunning) {
        if (cfg.isRunning) {
            log("applyBackendConfig -> enterAllowedMode");
            enterAllowedMode();
        }
        else {
            log("applyBackendConfig -> enterBlockedMode");
            enterBlockedMode();
        }
    }
}


void Controller::reloadConfig() {
    

    try {
        
        TimeConfig newcfg = configManager.load(configPath);
        log("reload success");
        bool wasRunning = cfg.isRunning;
        cfg.remainingMinutes = newcfg.remainingMinutes;
        cfg.isRunning = newcfg.isRunning;
        cfg.blockedApps = std::move(newcfg.blockedApps);
        cfg.blockedBrowserApps = std::move(newcfg.blockedBrowserApps);
        cfg.blockedWebsites = std::move(newcfg.blockedWebsites);
        

        if (wasRunning != cfg.isRunning) {
            if (cfg.isRunning) {
                log("reloadConfig - enterAllowedMode");
                enterAllowedMode();
            }
                
            else {
                log("reloadConfig - enterBlockedMode");
                enterBlockedMode();
            }
                
        }
    }
    catch (const std::exception& ex) {
        // LOG and ignore bad config
        std::cerr << "[Controller] Config reload failed: " << ex.what() << std::endl;
    }
    
}

void Controller::enterAllowedMode() {
    try {
        
        
        secondsCounter = 0;
        cfg.minutesToReduce = 0;
        hostsBlocker.removeBlock();
        log("enterAllowedMode - removeBlock");
    }
    
    catch (const std::exception& ex) {
        std::cerr << "[Controller] enterAllowedMode failed: "
            << ex.what() << std::endl;
        throw;
    }
}

void Controller::enterBlockedMode() {
    try {
        processManager.scanAndTerminate(cfg.blockedBrowserApps);
        log("enterBlockedMode");
        secondsCounter = 0;
        hostsBlocker.setBlockedWebsites(cfg.blockedWebsites);
        hostsBlocker.applyBlock();
        log("apply blocked websites");
        processManager.scanAndTerminate(cfg.blockedBrowserApps);
    }
    
    catch (const std::exception& ex) {
        std::cerr << "[Controller] enterBlockedMode failed: "
            << ex.what() << std::endl;

        // Roll back partial block
        try {
            hostsBlocker.removeBlock();
        }
        catch (const std::exception& rollbackEx) {
            std::cerr << "[Controller] Rollback failed: "
                << rollbackEx.what() << std::endl;
        }

        throw;
    }
}

void Controller::tickAllowedMode() {
    
    try {
        
        secondsCounter++;
        log("secondsCounter = " + std::to_string(secondsCounter));

        if (secondsCounter >= 60) {
           
            cfg.minutesToReduce++;
            secondsCounter = 0;
        }

        
    }
    catch (const std::exception& ex) {
        std::cerr << "[Controller] tickAllowedMode failed: " << ex.what() << std::endl;
    }
    
}

void Controller::tickBlockedMode() {
    try {
        processManager.scanAndTerminate(cfg.blockedApps);
    }
    
    catch (const std::exception& ex) {
        std::cerr << "[Controller] tickBlockedMode failed: " << ex.what() << std::endl;
    }
}
