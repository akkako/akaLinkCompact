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
    parseHWVersionString,
    parseFWVersionString,
    parseBLVersionString,
    parseHWProdDateString,
    parseFWCompileTimeString,
    parseBLCompileTimeString,
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
        
        // #region debug-point init
        // 绑定一次事件处理函数，避免重复添加
        this.boundHandleInputReport = this.handleInputReport.bind(this);
        this.boundHandleDisconnect = this.handleDisconnect.bind(this);
        // #endregion debug-point init
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
            
            // 等待设备就绪
            await sleep(100);

            // 设置输入报告回调
            // #region debug-point connect
            console.log('[DEBUG] 添加 inputreport 监听器, device.opened=', this.device.opened);
            // #endregion debug-point connect
            this.device.addEventListener('inputreport', this.boundHandleInputReport);

            // 监听设备断开事件
            navigator.hid.addEventListener('disconnect', this.boundHandleDisconnect);
            
            // #region debug-point connect
            console.log('[DEBUG] requestDevice 添加 disconnect 监听器');
            // #endregion debug-point connect

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
     * 设置已打开的设备（用于恢复已配对的设备）
     * @param {HIDDevice} device - 已打开的设备
     */
    async setDevice(device) {
        if (!device || !device.opened) {
            console.warn('设备未打开，无法设置');
            return;
        }

        // 如果已有设备，先断开
        if (this.device) {
            await this.disconnect();
        }

        this.device = device;

        // 等待设备就绪
        await sleep(100);

        // 设置输入报告回调
        this.device.addEventListener('inputreport', this.boundHandleInputReport);

        // 监听设备断开事件
        navigator.hid.addEventListener('disconnect', this.boundHandleDisconnect);

        if (this.onConnectCallback) {
            this.onConnectCallback(this.device);
        }
    }

    /**
     * 断开设备连接
     */
    async disconnect() {
        // #region debug-point disconnect
        console.log('[DEBUG] disconnect() 被调用, device=', this.device ? this.device.productName : null, 'opened=', this.device ? this.device.opened : null);
        // #endregion debug-point disconnect
        if (this.device) {
            const wasConnected = this.device.opened;
            const deviceToClose = this.device;

            // 移除 inputreport 监听器
            deviceToClose.removeEventListener('inputreport', this.boundHandleInputReport);
            
            this.device = null;
            navigator.hid.removeEventListener('disconnect', this.boundHandleDisconnect);
            
            // 清理 pendingResponse
            if (this.pendingResponse) {
                // #region debug-point disconnect
                console.log('[DEBUG] 清理 pendingResponse');
                // #endregion debug-point disconnect
                this.pendingResponse.reject(new Error('设备已断开连接'));
                this.pendingResponse = null;
            }

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
        // #region debug-point inputreport
        console.log('[DEBUG] handleInputReport 收到数据, reportId=', event.reportId, 'data[0]=', data[0].toString(16), 'pendingResponse=', this.pendingResponse ? '有' : '无');
        // #endregion debug-point inputreport
        
        // 如果有等待的响应，解析它
        if (this.pendingResponse) {
            // #region debug-point inputreport
            console.log('[DEBUG] 调用 pendingResponse.resolve');
            // #endregion debug-point inputreport
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
        // #region debug-point handleDisconnect
        console.log('[DEBUG] handleDisconnect 被调用, event.device=', event.device.productName);
        // #endregion debug-point handleDisconnect
        if (!this.device) return;
        
        // 检查是否是当前连接的设备断开
        const isSameDevice = event.device.productId === this.device.productId && 
                             event.device.vendorId === this.device.vendorId;
        
        // 对于某些设备，还需要检查序列号
        const isSameSerial = !event.device.serialNumber || !this.device.serialNumber || 
                             event.device.serialNumber === this.device.serialNumber;
        
        if (isSameDevice && isSameSerial) {
            // #region debug-point handleDisconnect
            console.log('[DEBUG] 检测到同一设备断开:', event.device.productName);
            // #endregion debug-point handleDisconnect
            this.device = null;
            navigator.hid.removeEventListener('disconnect', this.boundHandleDisconnect);
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
        // #region debug-point send
        console.log('[DEBUG] sendAndWaitResponse 被调用, device=', this.device ? this.device.productName : null, 'opened=', this.device ? this.device.opened : null, 'pendingResponse=', this.pendingResponse ? '有' : '无');
        // #endregion debug-point send
        if (!this.device || !this.device.opened) {
            throw new Error('设备未连接');
        }

        // 如果之前有未完成的响应，先清理
        if (this.pendingResponse) {
            // #region debug-point send
            console.log('[DEBUG] 清理之前的 pendingResponse');
            // #endregion debug-point send
            this.pendingResponse.reject(new Error('新的请求覆盖了旧请求'));
            this.pendingResponse = null;
        }

        // 创建等待响应的 Promise
        const responsePromise = new Promise((resolve, reject) => {
            this.pendingResponse = { resolve, reject };
            
            // 设置超时
            setTimeout(() => {
                if (this.pendingResponse) {
                    // #region debug-point send
                    console.log('[DEBUG] 响应超时');
                    // #endregion debug-point send
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
     * @param {number} reportId - 报告 ID（默认为 0，自动从设备描述符获取）
     * @returns {Promise<void>}
     */
    async sendOutputReport(data, reportId = 0) {
        if (!this.device) {
            throw new Error('设备未连接');
        }

        if (!this.device.opened) {
            throw new Error('设备连接已断开，请重新连接');
        }

        try {
            // 获取设备的报告信息
            const reportInfo = this.getDeviceReportInfo();
            
            // 如果没有指定 reportId，自动从描述符获取
            if (reportId === 0 && reportInfo) {
                reportId = reportInfo.outputReportId;
            }
            
            // 获取报告大小
            let reportSize = 64; // 默认 HID 报告大小
            if (reportInfo) {
                reportSize = reportInfo.outputReportSize;
            }

            // WebHID API 会自动处理 Report ID，所以数据部分不应该包含 Report ID
            // 数据格式: [Data Length][Command Type][Command Data...]
            // 跳过 data[0] (Report ID)，从 data[1] 开始发送
            let sendData = data.slice(1);
            
            // 确保数据长度为报告大小 - 1 (去掉 Report ID 后的长度)
            let paddedData = sendData;
            if (sendData.length < reportSize - 1) {
                paddedData = new Uint8Array(reportSize - 1);
                paddedData.set(sendData);
            } else if (sendData.length > reportSize - 1) {
                paddedData = sendData.slice(0, reportSize - 1);
            }

            console.log(`发送数据 - Report ID: ${reportId}, 数据长度: ${paddedData.length}`);
            console.log(`发送数据 - HEX:`, Array.from(paddedData).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' '));

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
     * 从 HID 集合中解析报告信息
     * @param {HIDCollectionInfo} collection - HID 集合信息
     * @returns {Object} 报告信息
     */
    parseReportInfo(collection) {
        const result = {
            outputReportId: 0,
            outputReportSize: 64,
            inputReportId: 0,
            inputReportSize: 64
        };

        if (!collection) return result;

        // 解析输出报告
        if (collection.outputReports && collection.outputReports.length > 0) {
            const outputReport = collection.outputReports[0];
            result.outputReportId = outputReport.reportId || 0;
            if (outputReport.items && outputReport.items.length > 0) {
                const totalBits = outputReport.items.reduce((sum, item) => {
                    return sum + ((item.reportCount || 1) * (item.reportSize || 8));
                }, 0);
                result.outputReportSize = Math.ceil(totalBits / 8);
            }
        }

        // 解析输入报告
        if (collection.inputReports && collection.inputReports.length > 0) {
            const inputReport = collection.inputReports[0];
            result.inputReportId = inputReport.reportId || 0;
            if (inputReport.items && inputReport.items.length > 0) {
                const totalBits = inputReport.items.reduce((sum, item) => {
                    return sum + ((item.reportCount || 1) * (item.reportSize || 8));
                }, 0);
                result.inputReportSize = Math.ceil(totalBits / 8);
            }
        }

        return result;
    }

    /**
     * 获取设备报告信息
     * @returns {Object} 报告信息
     */
    getDeviceReportInfo() {
        if (!this.device || !this.device.collections) {
            return {
                outputReportId: 0,
                outputReportSize: 64,
                inputReportId: 0,
                inputReportSize: 64
            };
        }

        const collections = this.device.collections;
        if (collections.length > 0) {
            return this.parseReportInfo(collections[0]);
        }

        return {
            outputReportId: 0,
            outputReportSize: 64,
            inputReportId: 0,
            inputReportSize: 64
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
        console.log('获取配置 - 原始响应数据:', Array.from(response).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' '));
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
        console.log('获取电压 - 原始响应数据:', Array.from(response).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' '));
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
        console.log('获取型号 - 原始响应数据:', Array.from(response).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' '));
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
        console.log('获取序列号 - 原始响应数据:', Array.from(response).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' '));
        const sn = parseSNString(response);
        if (!sn) {
            throw new Error('获取序列号失败：响应数据无效');
        }
        return sn;
    }

    /**
     * 获取硬件版本号
     * @returns {Promise<string>} 硬件版本号字符串
     */
    async getHWVersion() {
        const packet = createHIDPacket(0x01, 0x01, 0x12);
        const response = await this.sendAndWaitResponse(packet);
        console.log('获取硬件版本号 - 原始响应数据:', Array.from(response).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' '));
        const hw = parseHWVersionString(response);
        if (!hw) {
            throw new Error('获取硬件版本号失败：响应数据无效');
        }
        return hw;
    }

    /**
     * 获取固件版本号
     * @returns {Promise<string>} 固件版本号字符串
     */
    async getFWVersion() {
        const packet = createHIDPacket(0x01, 0x01, 0x13);
        const response = await this.sendAndWaitResponse(packet);
        console.log('获取固件版本号 - 原始响应数据:', Array.from(response).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' '));
        const fw = parseFWVersionString(response);
        if (!fw) {
            throw new Error('获取固件版本号失败：响应数据无效');
        }
        return fw;
    }

    /**
     * 获取Bootloader版本号
     * @returns {Promise<string>} Bootloader版本号字符串
     */
    async getBLVersion() {
        const packet = createHIDPacket(0x01, 0x01, 0x14);
        const response = await this.sendAndWaitResponse(packet);
        console.log('获取Bootloader版本号 - 原始响应数据:', Array.from(response).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' '));
        const bl = parseBLVersionString(response);
        if (!bl) {
            throw new Error('获取Bootloader版本号失败：响应数据无效');
        }
        return bl;
    }

    /**
     * 获取硬件生产日期
     * @returns {Promise<string>} 硬件生产日期字符串
     */
    async getHWProdDate() {
        const packet = createHIDPacket(0x01, 0x01, 0x15);
        const response = await this.sendAndWaitResponse(packet);
        console.log('获取硬件生产日期 - 原始响应数据:', Array.from(response).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' '));
        const hw = parseHWProdDateString(response);
        if (!hw) {
            throw new Error('获取硬件生产日期失败：响应数据无效');
        }
        return hw;
    }

    /**
     * 获取固件编译时间
     * @returns {Promise<string>} 固件编译时间字符串
     */
    async getFWCompileTime() {
        const packet = createHIDPacket(0x01, 0x01, 0x16);
        const response = await this.sendAndWaitResponse(packet);
        console.log('获取固件编译时间 - 原始响应数据:', Array.from(response).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' '));
        const fw = parseFWCompileTimeString(response);
        if (!fw) {
            throw new Error('获取固件编译时间失败：响应数据无效');
        }
        return fw;
    }

    /**
     * 获取Bootloader编译时间
     * @returns {Promise<string>} Bootloader编译时间字符串
     */
    async getBLCompileTime() {
        const packet = createHIDPacket(0x01, 0x01, 0x17);
        const response = await this.sendAndWaitResponse(packet);
        console.log('获取Bootloader编译时间 - 原始响应数据:', Array.from(response).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' '));
        const bl = parseBLCompileTimeString(response);
        if (!bl) {
            throw new Error('获取Bootloader编译时间失败：响应数据无效');
        }
        return bl;
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
