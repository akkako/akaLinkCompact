/**
 * 应用程序入口模块
 * akaLink CMSIS-DAP 配置工具
 */

import { DeviceManager } from './device-manager.js';
import { isWebHIDSupported, formatTime } from './utils.js';

/**
 * 应用程序主类
 */
class akaLinkConfigApp {
    constructor() {
        // 检查浏览器支持
        if (!isWebHIDSupported()) {
            this.showBrowserNotSupported();
            return;
        }

        // 初始化设备管理器
        this.deviceManager = new DeviceManager();
        
        // 当前配置状态
        this.currentConfig = null;
        this.originalConfig = null;
        this.isConfigModified = false;

        // 电压更新定时器
        this.voltageTimer = null;

        // 绑定 UI 元素
        this.bindElements();
        
        // 绑定事件
        this.bindEvents();
        
        // 设置设备管理器回调
        this.setupCallbacks();

        // 尝试恢复已配对的设备
        this.restorePairedDevices();
    }

    /**
     * 绑定 UI 元素
     */
    bindElements() {
        // 连接控制
        this.connectBtn = document.getElementById('connectBtn');
        this.disconnectBtn = document.getElementById('disconnectBtn');
        this.connectionStatus = document.getElementById('connectionStatus');

        // 设备信息
        this.deviceModel = document.getElementById('deviceModel');
        this.deviceSN = document.getElementById('deviceSN');
        this.targetVoltage = document.getElementById('targetVoltage');

        // 配置项
        this.outputMode = document.getElementById('outputMode');
        this.swdMode = document.getElementById('swdMode');
        this.v5Mode = document.getElementById('v5Mode');
        this.clockMode = document.getElementById('clockMode');
        this.configNotice = document.getElementById('configNotice');

        // 操作按钮
        this.saveConfigBtn = document.getElementById('saveConfigBtn');
        this.resetBtn = document.getElementById('resetBtn');
        this.dfuBtn = document.getElementById('dfuBtn');

        // 日志
        this.logOutput = document.getElementById('logOutput');
        this.clearLogBtn = document.getElementById('clearLogBtn');

        // 模态框
        this.confirmModal = document.getElementById('confirmModal');
        this.modalTitle = document.getElementById('modalTitle');
        this.modalMessage = document.getElementById('modalMessage');
        this.modalCancel = document.getElementById('modalCancel');
        this.modalConfirm = document.getElementById('modalConfirm');
    }

    /**
     * 绑定事件
     */
    bindEvents() {
        // 连接控制
        this.connectBtn.addEventListener('click', () => this.handleConnect());
        this.disconnectBtn.addEventListener('click', () => this.handleDisconnect());

        // 配置变更监听
        this.outputMode.addEventListener('change', () => this.onConfigChanged());
        this.swdMode.addEventListener('change', () => this.onConfigChanged());
        this.v5Mode.addEventListener('change', () => this.onConfigChanged());
        this.clockMode.addEventListener('change', () => this.onConfigChanged());

        // 操作按钮
        this.saveConfigBtn.addEventListener('click', () => this.handleSaveConfig());
        this.resetBtn.addEventListener('click', () => this.handleReset());
        this.dfuBtn.addEventListener('click', () => this.handleDFU());

        // 日志
        this.clearLogBtn.addEventListener('click', () => this.clearLog());

        // 模态框
        this.modalCancel.addEventListener('click', () => this.hideModal());
    }

    /**
     * 设置设备管理器回调
     */
    setupCallbacks() {
        // 设备连接回调
        this.deviceManager.setOnConnect((device) => {
            this.log(`已连接到设备: ${device.productName}`, 'success');
            this.updateConnectionStatus(true);
            this.readDeviceInfo();
            this.startVoltageUpdate();
        });

        // 设备断开回调
        this.deviceManager.setOnDisconnect(() => {
            this.log('设备已断开连接', 'warning');
            this.stopVoltageUpdate();
            this.updateConnectionStatus(false);
            this.clearDeviceInfo();
        });
    }

    /**
     * 启动电压定时更新
     */
    startVoltageUpdate() {
        // 先停止之前的定时器
        this.stopVoltageUpdate();
        // 每 2 秒更新一次电压
        this.voltageTimer = setInterval(() => {
            this.readVoltage();
        }, 2000);
    }

    /**
     * 停止电压定时更新
     */
    stopVoltageUpdate() {
        if (this.voltageTimer) {
            clearInterval(this.voltageTimer);
            this.voltageTimer = null;
        }
    }

    /**
     * 尝试恢复已配对的设备
     */
    async restorePairedDevices() {
        try {
            const devices = await this.deviceManager.getPairedDevices();
            for (const device of devices) {
                if (device.opened) {
                    // 使用设备管理器的 setDevice 方法来正确设置设备
                    await this.deviceManager.setDevice(device);
                    this.log('已恢复设备连接', 'success');
                    break;
                }
            }
        } catch (error) {
            console.error('恢复设备连接失败:', error);
        }
    }

    // ==================== 事件处理 ====================

    /**
     * 处理连接按钮点击
     */
    async handleConnect() {
        try {
            this.log('正在请求连接设备...', 'info');
            await this.deviceManager.requestDevice();
        } catch (error) {
            this.log(`连接失败: ${error.message}`, 'error');
        }
    }

    /**
     * 处理断开按钮点击
     */
    async handleDisconnect() {
        try {
            await this.deviceManager.disconnect();
        } catch (error) {
            this.log(`断开连接失败: ${error.message}`, 'error');
        }
    }

    /**
     * 处理保存配置
     */
    async handleSaveConfig() {
        if (!this.isConfigModified) {
            this.log('配置未修改', 'info');
            return;
        }

        try {
            this.log('正在写入配置...', 'info');
            
            // 写入配置
            const config = {
                outputMode: parseInt(this.outputMode.value),
                swdMode: parseInt(this.swdMode.value),
                v5Mode: parseInt(this.v5Mode.value),
                clockMode: parseInt(this.clockMode.value)
            };
            
            await this.deviceManager.setConfig(config);
            this.log('配置已写入', 'success');

            // 保存配置
            this.log('正在保存配置到设备...', 'info');
            await this.deviceManager.saveConfig();
            this.log('配置已保存', 'success');

            // 更新状态
            this.originalConfig = { ...config };
            this.isConfigModified = false;
            this.updateConfigNotice();

            // 刷新电压显示
            this.readVoltage();

        } catch (error) {
            this.log(`保存配置失败: ${error.message}`, 'error');
        }
    }

    /**
     * 处理复位按钮点击
     */
    handleReset() {
        this.showModal(
            '确认复位',
            '确定要复位设备吗？设备将重新启动。',
            async () => {
                this.hideModal();
                try {
                    this.log('正在复位设备...', 'info');
                    await this.deviceManager.resetDevice();
                    this.log('设备已复位', 'success');
                } catch (error) {
                    this.log(`复位失败: ${error.message}`, 'error');
                }
            }
        );
    }

    /**
     * 处理 DFU 按钮点击
     */
    handleDFU() {
        this.showModal(
            '确认进入 DFU 模式',
            '确定要进入 DFU 模式吗？设备将进入固件升级模式并断开连接。',
            async () => {
                this.hideModal();
                try {
                    this.log('正在进入 DFU 模式...', 'info');
                    await this.deviceManager.enterDFU();
                    this.log('设备已进入 DFU 模式', 'success');
                } catch (error) {
                    this.log(`进入 DFU 模式失败: ${error.message}`, 'error');
                }
            }
        );
    }

    /**
     * 配置变更处理
     */
    onConfigChanged() {
        if (!this.originalConfig) return;

        const currentConfig = {
            outputMode: parseInt(this.outputMode.value),
            swdMode: parseInt(this.swdMode.value),
            v5Mode: parseInt(this.v5Mode.value),
            clockMode: parseInt(this.clockMode.value)
        };

        this.isConfigModified = 
            currentConfig.outputMode !== this.originalConfig.outputMode ||
            currentConfig.swdMode !== this.originalConfig.swdMode ||
            currentConfig.v5Mode !== this.originalConfig.v5Mode ||
            currentConfig.clockMode !== this.originalConfig.clockMode;

        this.updateConfigNotice();
    }

    // ==================== 设备信息读取 ====================

    /**
     * 读取设备信息
     */
    async readDeviceInfo() {
        try {
            // 读取型号
            this.log('正在读取型号...', 'info');
            const model = await this.deviceManager.getModel();
            this.deviceModel.textContent = model;
            this.log(`型号: ${model}`, 'success');

            // 读取序列号
            this.log('正在读取序列号...', 'info');
            const sn = await this.deviceManager.getSN();
            this.deviceSN.textContent = sn;
            this.log(`序列号: ${sn}`, 'success');

            // 读取配置
            this.log('正在读取配置...', 'info');
            const config = await this.deviceManager.getConfig();
            this.originalConfig = { ...config };
            this.applyConfigToUI(config);
            this.log('配置已读取', 'success');

            // 读取电压
            this.readVoltage();

            // 启用配置控件
            this.setConfigControlsEnabled(true);

        } catch (error) {
            this.log(`读取设备信息失败: ${error.message}`, 'error');
        }
    }

    /**
     * 读取电压
     */
    async readVoltage() {
        try {
            const voltage = await this.deviceManager.getVoltage();
            this.targetVoltage.textContent = `${voltage} mV (${(voltage / 1000).toFixed(2)} V)`;
        } catch (error) {
            this.targetVoltage.textContent = '--';
            console.error('读取电压失败:', error);
        }
    }

    // ==================== UI 更新 ====================

    /**
     * 更新连接状态显示
     * @param {boolean} connected - 是否已连接
     */
    updateConnectionStatus(connected) {
        if (connected) {
            this.connectionStatus.textContent = '已连接';
            this.connectionStatus.className = 'status-connected';
            this.connectBtn.disabled = true;
            this.disconnectBtn.disabled = false;
            this.saveConfigBtn.disabled = false;
            this.resetBtn.disabled = false;
            this.dfuBtn.disabled = false;
        } else {
            this.connectionStatus.textContent = '未连接';
            this.connectionStatus.className = 'status-disconnected';
            this.connectBtn.disabled = false;
            this.disconnectBtn.disabled = true;
            this.saveConfigBtn.disabled = true;
            this.resetBtn.disabled = true;
            this.dfuBtn.disabled = true;
            this.setConfigControlsEnabled(false);
        }
    }

    /**
     * 清除设备信息显示
     */
    clearDeviceInfo() {
        this.deviceModel.textContent = '--';
        this.deviceSN.textContent = '--';
        this.targetVoltage.textContent = '--';
        this.outputMode.value = '0';
        this.swdMode.value = '0';
        this.v5Mode.value = '0';
        this.clockMode.value = '0';
        this.originalConfig = null;
        this.isConfigModified = false;
        this.updateConfigNotice();
    }

    /**
     * 应用配置到 UI
     * @param {Object} config - 配置对象
     */
    applyConfigToUI(config) {
        this.outputMode.value = config.outputMode.toString();
        this.swdMode.value = config.swdMode.toString();
        this.v5Mode.value = config.v5Mode.toString();
        this.clockMode.value = config.clockMode.toString();
    }

    /**
     * 设置配置控件启用状态
     * @param {boolean} enabled - 是否启用
     */
    setConfigControlsEnabled(enabled) {
        this.outputMode.disabled = !enabled;
        this.swdMode.disabled = !enabled;
        this.v5Mode.disabled = !enabled;
        this.clockMode.disabled = !enabled;
    }

    /**
     * 更新配置修改提示
     */
    updateConfigNotice() {
        this.configNotice.style.display = this.isConfigModified ? 'flex' : 'none';
    }

    // ==================== 日志功能 ====================

    /**
     * 添加日志
     * @param {string} message - 日志消息
     * @param {string} type - 日志类型 (info, success, error, warning)
     */
    log(message, type = 'info') {
        const entry = document.createElement('div');
        entry.className = `log-entry ${type}`;
        entry.textContent = `[${formatTime(new Date())}] ${message}`;
        this.logOutput.appendChild(entry);
        this.logOutput.scrollTop = this.logOutput.scrollHeight;
    }

    /**
     * 清空日志
     */
    clearLog() {
        this.logOutput.innerHTML = '<div class="log-entry info">日志已清空</div>';
    }

    // ==================== 模态框 ====================

    /**
     * 显示确认模态框
     * @param {string} title - 标题
     * @param {string} message - 消息
     * @param {Function} onConfirm - 确认回调
     */
    showModal(title, message, onConfirm) {
        this.modalTitle.textContent = title;
        this.modalMessage.textContent = message;
        this.confirmModal.style.display = 'flex';
        
        // 移除旧的事件监听器
        const newConfirmBtn = this.modalConfirm.cloneNode(true);
        this.modalConfirm.parentNode.replaceChild(newConfirmBtn, this.modalConfirm);
        this.modalConfirm = newConfirmBtn;
        
        this.modalConfirm.addEventListener('click', onConfirm);
    }

    /**
     * 隐藏模态框
     */
    hideModal() {
        this.confirmModal.style.display = 'none';
    }

    // ==================== 浏览器支持检查 ====================

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
                font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
                text-align: center;
                padding: 20px;
                background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
                color: #e0e0e0;
            ">
                <h1 style="color: #e74c3c; margin-bottom: 20px;">浏览器不支持</h1>
                <p style="color: #8892b0; max-width: 600px; line-height: 1.6;">
                    您的浏览器不支持 WebHID API。请使用以下浏览器之一：
                </p>
                <ul style="color: #8892b0; text-align: left; margin: 20px 0; line-height: 1.8;">
                    <li>Google Chrome 89+</li>
                    <li>Microsoft Edge 89+</li>
                    <li>Opera 75+</li>
                </ul>
                <p style="color: #00d4ff; font-size: 14px;">
                    WebHID 允许网页与 HID 设备通信，但需要浏览器的安全支持。
                </p>
            </div>
        `;
    }
}

// 页面加载完成后初始化应用
document.addEventListener('DOMContentLoaded', () => {
    window.akaLinkApp = new akaLinkConfigApp();
});
