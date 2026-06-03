/**
 * 应用程序入口模块
 * 初始化所有模块并建立它们之间的关联
 */

import { DeviceManager } from './device-manager.js';
import { SendManager } from './send-manager.js';
import { ReceiveManager } from './receive-manager.js';
import { UIController } from './ui-controller.js';
import { isWebHIDSupported } from './utils.js';

/**
 * 应用程序主类
 */
class HIDDebuggerApp {
    constructor() {
        // 检查浏览器支持
        if (!isWebHIDSupported()) {
            this.showBrowserNotSupported();
            return;
        }

        // 初始化各个管理器
        this.deviceManager = new DeviceManager();
        this.sendManager = new SendManager();
        this.receiveManager = new ReceiveManager();
        
        // 初始化 UI 控制器
        this.uiController = new UIController(
            this.deviceManager,
            this.sendManager,
            this.receiveManager
        );

        // 建立模块间的回调关系
        this.setupCallbacks();

        // 尝试恢复已配对的设备
        this.restorePairedDevices();
    }

    /**
     * 设置各个模块间的回调函数
     */
    setupCallbacks() {
        // 设备连接回调
        this.deviceManager.setOnConnect((device) => {
            console.log('设备已连接:', device.productName);
            this.uiController.updateConnectionStatus(true);
            this.uiController.updateSendButtonsState();
            this.uiController.showSuccess(`已连接到设备: ${device.productName}`);
        });

        // 设备断开回调
        this.deviceManager.setOnDisconnect(() => {
            console.log('设备已断开');
            this.uiController.updateConnectionStatus(false);
            this.uiController.updateSendButtonsState();
        });

        // 输入报告回调（接收数据）
        this.deviceManager.setOnInputReport((event) => {
            this.receiveManager.addReceivedData(event.data, event.timestamp);
        });

        // 发送回调
        // rawData - 原始数据
        // dataWithHeader - 带长度头的数据（用于实际发送）
        // headerBytes - 长度头字节数
        this.sendManager.setOnSend(async (entry, rawData, dataWithHeader, sendMode, reportId, headerBytes) => {
            try {
                // 根据发送模式选择发送方法（使用带长度头的数据）
                if (sendMode === 'feature') {
                    await this.deviceManager.sendFeatureReport(dataWithHeader, reportId);
                } else {
                    await this.deviceManager.sendOutputReport(dataWithHeader, reportId);
                }

                // 记录发送的数据（包含长度头，显示时根据格式处理）
                this.receiveManager.addSentData(dataWithHeader, new Date(), headerBytes);

                console.log(`发送条目 #${entry.id}: 原始数据 ${rawData.length} 字节, 实际发送 ${dataWithHeader.length} 字节, 模式: ${sendMode}, Report ID: ${reportId}`);
            } catch (error) {
                console.error('发送失败:', error);
                throw error;
            }
        });

        // 接收数据更新回调
        this.receiveManager.setOnUpdate(() => {
            this.uiController.updateReceiveDisplay();
        });
    }

    /**
     * 尝试恢复已配对的设备
     */
    async restorePairedDevices() {
        try {
            const devices = await this.deviceManager.getPairedDevices();
            
            // 如果有已配对且已打开的设备，自动连接
            for (const device of devices) {
                if (device.opened) {
                    this.deviceManager.device = device;
                    
                    // 设置输入报告监听
                    device.addEventListener('inputreport', (event) => {
                        this.deviceManager.handleInputReport(event);
                    });
                    
                    // 更新 UI
                    this.uiController.updateConnectionStatus(true);
                    this.uiController.updateSendButtonsState();
                    
                    console.log('已恢复设备连接:', device.productName);
                    break;
                }
            }
        } catch (error) {
            console.error('恢复设备连接失败:', error);
        }
    }

    /**
     * 显示浏览器不支持提示
     */
    showBrowserNotSupported() {
        document.body.innerHTML = `
            <div style="
                display: flex;
                flex-direction: column;
                align-items: center;
                justify-content: center;
                height: 100vh;
                font-family: Arial, sans-serif;
                text-align: center;
                padding: 20px;
            ">
                <h1 style="color: #e74c3c; margin-bottom: 20px;">浏览器不支持</h1>
                <p style="color: #555; max-width: 600px; line-height: 1.6;">
                    您的浏览器不支持 WebHID API。请使用以下浏览器之一：
                </p>
                <ul style="color: #555; text-align: left; margin: 20px 0;">
                    <li>Google Chrome 89+</li>
                    <li>Microsoft Edge 89+</li>
                    <li>Opera 75+</li>
                </ul>
                <p style="color: #888; font-size: 14px;">
                    WebHID 允许网页与 HID 设备通信，但需要浏览器的安全支持。
                </p>
            </div>
        `;
    }
}

// 页面加载完成后初始化应用
document.addEventListener('DOMContentLoaded', () => {
    window.hidDebuggerApp = new HIDDebuggerApp();
    initThemeToggle();
});

/**
 * 初始化主题切换功能
 */
function initThemeToggle() {
    const themeToggle = document.getElementById('themeToggle');
    const themeIcon = document.getElementById('themeIcon');
    const body = document.body;
    
    // 统一的 localStorage key
    const THEME_KEY = 'akalink-theme';
    
    // 获取保存的主题或检测系统主题
    function getInitialTheme() {
        const savedTheme = localStorage.getItem(THEME_KEY);
        if (savedTheme) {
            return savedTheme;
        }
        // 检测系统主题 - 默认深色
        if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
            return 'dark';
        }
        return 'light';
    }
    
    // 应用主题
    function applyTheme(theme) {
        if (theme === 'light') {
            body.classList.remove('dark-theme');
            themeIcon.textContent = '☀️';
        } else {
            body.classList.add('dark-theme');
            themeIcon.textContent = '🌙';
        }
        localStorage.setItem(THEME_KEY, theme);
    }
    
    // 切换主题
    function toggleTheme() {
        // 添加动画类
        themeToggle.classList.add('animating');
        
        setTimeout(() => {
            themeToggle.classList.remove('animating');
        }, 500);
        
        const isDark = body.classList.contains('dark-theme');
        const newTheme = isDark ? 'light' : 'dark';
        applyTheme(newTheme);
    }
    
    // 初始化
    applyTheme(getInitialTheme());
    
    // 监听点击
    themeToggle.addEventListener('click', toggleTheme);
    
    // 监听系统主题变化
    if (window.matchMedia) {
        window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', (e) => {
            // 只有在用户没有手动设置主题时才自动切换
            if (!localStorage.getItem(THEME_KEY)) {
                applyTheme(e.matches ? 'dark' : 'light');
            }
        });
    }
    
    // 监听其他页面修改主题（通过 localStorage 同步）
    window.addEventListener('storage', (e) => {
        if (e.key === THEME_KEY) {
            const newTheme = e.newValue || 'dark';
            applyTheme(newTheme);
        }
    });
}
