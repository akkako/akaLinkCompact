/**
 * 设备管理模块
 * 负责 WebHID 设备的连接、断开和通信
 */

import {
    isWebHIDSupported,
    createHIDPacket,
    parseConfigData,
    parseVoltageData,
    parseModelString,
    parseSNString,
    isResponseSuccess,
    sleep
} from './utils.js';

/**
 * 设备管理器类
 */
export class DeviceManager {
    constructor() {
        this.device = null;
        this.onConnectCallback = null;
        this.onDisconnectCallback = null;
        this.onInputReportCallback = null;
        this.pendingResponse = null;
        this.responseTimeout = 5000; // 5秒超时
    }

    /**
     * 设置连接回调
     * @param {Function} callback - 连接成功时的回调函数
     */
    setOnConnect(callback) {
        this.onConnectCallback = callback;
    }

    /**
     * 设置断开连接回调
     * @param {Function} callback - 断开连接时的回调函数
     */
    setOnDisconnect(callback) {
        this.onDisconnectCallback = callback;
    }

    /**
     * 设置输入报告回调
     * @param {Function} callback - 收到输入报告时的回调函数
     */
    setOnInputReport(callback) {
        this.onInputReportCallback = callback;
    }

    /**
     * 检查是否支持 WebHID
     * @returns {boolean} 是否支持
     */
    isSupported() {
        return isWebHIDSupported();
    }

    /**
     * 请求连接 HID 设备
     * @returns {Promise<HIDDevice>} 连接的设备
     */
    async requestDevice() {
        if (!this.isSupported()) {
            throw new Error('当前浏览器不支持 WebHID API');
        }

        try {
            // 如果已有设备连接，先断开
            if (this.device) {
                await this.disconnect();
            }

            // 请求用户选择 HID 设备
            const devices = await navigator.hid.requestDevice({
                filters: [] // 空过滤器显示所有设备
            });

            if (devices.length === 0) {
                throw new Error('未选择任何设备');
            }

            this.device = devices[0];

            // 如果设备已经打开，先关闭再重新打开
            if (this.device.opened) {
                try {
                    await this.device.close();
                    await sleep(100);
                } catch (error) {
                    console.warn('关闭已打开的设备时出错:', error);
                }
            }

            // 打开设备
            await this.device.open();

            // 设置输入报告回调
            this.device.addEventListener('inputreport', this.handleInputReport.bind(this));

            // 监听设备断开事件
            navigator.hid.addEventListener('disconnect', this.handleDisconnect.bind(this));

            if (this.onConnectCallback) {
                this.onConnectCallback(this.device);
            }

            return this.device;
        } catch (error) {
            console.error('连接设备失败:', error);
            this.device = null;
            throw error;
        }
    }

    /**
     * 断开设备连接
     */
    async disconnect() {
        if (this.device) {
            const wasConnected = this.device.opened;
            const deviceToClose = this.device;

            this.device = null;
            navigator.hid.removeEventListener('disconnect', this.handleDisconnect);

            try {
                if (wasConnected && deviceToClose) {
                    await deviceToClose.close();
                }
            } catch (error) {
                console.error('关闭设备时出错:', error);
            }

            if (this.onDisconnectCallback) {
                this.onDisconnectCallback();
            }
        }
    }

    /**
     * 处理输入报告
     * @param {HIDInputReportEvent} event - 输入报告事件
     */
    handleInputReport(event) {
        const data = new Uint8Array(event.data.buffer);
        
        // 如果有等待的响应，解析它
        if (this.pendingResponse) {
            this.pendingResponse.resolve(data);
            this.pendingResponse = null;
        }

        if (this.onInputReportCallback) {
            this.onInputReportCallback(data);
        }
    }

    /**
     * 处理设备断开
     * @param {HIDConnectionEvent} event - 连接事件
     */
    handleDisconnect(event) {
        if (this.device && 
            event.device.productId === this.device.productId && 
            event.device.vendorId === this.device.vendorId) {
            this.device = null;
            if (this.onDisconnectCallback) {
                this.onDisconnectCallback();
            }
        }
    }

    /**
     * 发送数据并等待响应
     * @param {Uint8Array} data - 要发送的数据
     * @param {number} timeout - 超时时间（毫秒）
     * @returns {Promise<Uint8Array>} 响应数据
     */
    async sendAndWaitResponse(data, timeout = this.responseTimeout) {
        if (!this.device || !this.device.opened) {
            throw new Error('设备未连接');
        }

        // 创建等待响应的 Promise
        const responsePromise = new Promise((resolve, reject) => {
            this.pendingResponse = { resolve, reject };
            
            // 设置超时
            setTimeout(() => {
                if (this.pendingResponse) {
                    this.pendingResponse = null;
                    reject(new Error('设备响应超时'));
                }
            }, timeout);
        });

        // 发送数据
        await this.sendOutputReport(data);

        return responsePromise;
    }

    /**
     * 发送输出报告
     * @param {Uint8Array} data - 要发送的数据
     * @param {number} reportId - 报告 ID（默认为 1）
     * @returns {Promise<void>}
     */
    async sendOutputReport(data, reportId = 1) {
        if (!this.device) {
            throw new Error('设备未连接');
        }

        if (!this.device.opened) {
            try {
                await this.device.open();
            } catch (error) {
                throw new Error('设备连接已断开，请重新连接');
            }
        }

        try {
            // 确保数据长度为64字节
            let paddedData = data;
            if (data.length < 64) {
                paddedData = new Uint8Array(64);
                paddedData.set(data);
            } else if (data.length > 64) {
                paddedData = data.slice(0, 64);
            }

            await this.device.sendReport(reportId, paddedData);
        } catch (error) {
            console.error('发送数据失败:', error);
            
            if (error.name === 'NotAllowedError') {
                this.device = null;
                if (this.onDisconnectCallback) {
                    this.onDisconnectCallback();
                }
                throw new Error('设备权限已丢失，请重新连接设备');
            } else if (error.name === 'NotFoundError') {
                this.device = null;
                if (this.onDisconnectCallback) {
                    this.onDisconnectCallback();
                }
                throw new Error('设备未找到，可能已断开连接');
            }
            throw new Error(`发送失败: ${error.message || '未知错误'}`);
        }
    }

    /**
     * 获取设备信息
     * @returns {Object|null} 设备信息对象
     */
    getDeviceInfo() {
        if (!this.device) {
            return null;
        }

        return {
            vendorId: this.device.vendorId,
            productId: this.device.productId,
            productName: this.device.productName,
            opened: this.device.opened
        };
    }

    /**
     * 检查设备是否已连接
     * @returns {boolean} 是否已连接
     */
    isConnected() {
        return this.device !== null && this.device.opened;
    }

    /**
     * 获取已配对的设备
     * @returns {Promise<HIDDevice[]>}
     */
    async getPairedDevices() {
        if (!this.isSupported()) {
            return [];
        }

        try {
            const devices = await navigator.hid.getDevices();
            return devices;
        } catch (error) {
            console.error('获取已配对设备失败:', error);
            return [];
        }
    }

    // ==================== 协议命令封装 ====================

    /**
     * 获取配置
     * @returns {Promise<Object>} 配置对象
     */
    async getConfig() {
        const packet = createHIDPacket(0x01, 0x01, 0x01);
        const response = await this.sendAndWaitResponse(packet);
        const config = parseConfigData(response);
        if (!config) {
            throw new Error('获取配置失败：响应数据无效');
        }
        return config;
    }

    /**
     * 设置配置
     * @param {Object} config - 配置对象
     * @returns {Promise<boolean>} 是否成功
     */
    async setConfig(config) {
        const data = new Uint8Array([
            config.outputMode,
            config.swdMode,
            config.v5Mode,
            config.clockMode
        ]);
        const packet = createHIDPacket(0x01, 0x05, 0x02, data);
        const response = await this.sendAndWaitResponse(packet);
        return isResponseSuccess(response, 0x02);
    }

    /**
     * 获取目标电压
     * @returns {Promise<number>} 电压值(mV)
     */
    async getVoltage() {
        const packet = createHIDPacket(0x01, 0x01, 0x03);
        const response = await this.sendAndWaitResponse(packet);
        const voltage = parseVoltageData(response);
        if (voltage === null) {
            throw new Error('获取电压失败：响应数据无效');
        }
        return voltage;
    }

    /**
     * 保存配置
     * @returns {Promise<boolean>} 是否成功
     */
    async saveConfig() {
        const packet = createHIDPacket(0x01, 0x01, 0x04);
        const response = await this.sendAndWaitResponse(packet);
        return isResponseSuccess(response, 0x04);
    }

    /**
     * 获取型号
     * @returns {Promise<string>} 型号字符串
     */
    async getModel() {
        const packet = createHIDPacket(0x01, 0x01, 0x10);
        const response = await this.sendAndWaitResponse(packet);
        const model = parseModelString(response);
        if (!model) {
            throw new Error('获取型号失败：响应数据无效');
        }
        return model;
    }

    /**
     * 获取序列号
     * @returns {Promise<string>} 序列号字符串
     */
    async getSN() {
        const packet = createHIDPacket(0x01, 0x01, 0x11);
        const response = await this.sendAndWaitResponse(packet);
        const sn = parseSNString(response);
        if (!sn) {
            throw new Error('获取序列号失败：响应数据无效');
        }
        return sn;
    }

    /**
     * 复位设备
     * @returns {Promise<void>}
     */
    async resetDevice() {
        const packet = createHIDPacket(0x01, 0x01, 0xFE);
        await this.sendOutputReport(packet);
        // 复位后设备会断开，不需要等待响应
        await sleep(100);
        this.device = null;
        if (this.onDisconnectCallback) {
            this.onDisconnectCallback();
        }
    }

    /**
     * 进入 DFU 模式
     * @returns {Promise<void>}
     */
    async enterDFU() {
        const packet = createHIDPacket(0x01, 0x01, 0xFF);
        await this.sendOutputReport(packet);
        // 进入 DFU 后设备会断开，不需要等待响应
        await sleep(100);
        this.device = null;
        if (this.onDisconnectCallback) {
            this.onDisconnectCallback();
        }
    }
}
