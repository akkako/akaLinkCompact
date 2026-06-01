/**
 * 设备管理模块
 * 负责 WebHID 设备的连接、断开和信息获取
 */

import { isWebHIDSupported } from './utils.js';

/**
 * 设备管理器类
 */
export class DeviceManager {
    constructor() {
        this.device = null;
        this.onConnectCallback = null;
        this.onDisconnectCallback = null;
        this.onInputReportCallback = null;
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
     * @param {Function} callback - 收到输入报告时的回调函数 (event) => {}
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
                console.log('断开之前的连接...');
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

            // 如果设备已经打开（之前连接过），先关闭再重新打开
            if (this.device.opened) {
                console.log('设备已打开，先关闭再重新打开...');
                try {
                    await this.device.close();
                    // 等待一小段时间确保设备完全关闭
                    await new Promise(resolve => setTimeout(resolve, 100));
                } catch (error) {
                    console.warn('关闭已打开的设备时出错:', error);
                }
            }

            // 打开设备
            console.log('正在打开设备...');
            await this.device.open();
            console.log('设备已打开');

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
            // 清理设备引用
            this.device = null;
            throw error;
        }
    }

    /**
     * 断开设备连接
     */
    async disconnect() {
        if (this.device) {
            // 记录连接状态（在关闭前检查）
            const wasConnected = this.device.opened;
            const deviceToClose = this.device;

            // 先清除设备引用，防止后续操作使用已断开的设备
            this.device = null;

            // 移除全局断开事件监听
            navigator.hid.removeEventListener('disconnect', this.handleDisconnect);

            try {
                if (wasConnected && deviceToClose) {
                    console.log('正在关闭设备...');
                    await deviceToClose.close();
                    console.log('设备已关闭');
                }
            } catch (error) {
                console.error('关闭设备时出错:', error);
            }

            // 无论之前是否已连接，都触发断开回调以更新UI
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
        if (this.onInputReportCallback) {
            this.onInputReportCallback({
                reportId: event.reportId,
                data: new Uint8Array(event.data.buffer),
                timestamp: new Date()
            });
        }
    }

    /**
     * 处理设备断开
     * @param {HIDConnectionEvent} event - 连接事件
     */
    handleDisconnect(event) {
        if (this.device && event.device.productId === this.device.productId && 
            event.device.vendorId === this.device.vendorId) {
            this.device = null;
            if (this.onDisconnectCallback) {
                this.onDisconnectCallback();
            }
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
     * @returns {Object} 报告信息 { reportId, reportSize }
     */
    parseReportInfo(collection) {
        const result = {
            outputReportId: 0,
            outputReportSize: 64,
            inputReportId: 0,
            inputReportSize: 64,
            featureReportId: 0,
            featureReportSize: 64
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

        // 解析特性报告
        if (collection.featureReports && collection.featureReports.length > 0) {
            const featureReport = collection.featureReports[0];
            result.featureReportId = featureReport.reportId || 0;
            if (featureReport.items && featureReport.items.length > 0) {
                const totalBits = featureReport.items.reduce((sum, item) => {
                    return sum + ((item.reportCount || 1) * (item.reportSize || 8));
                }, 0);
                result.featureReportSize = Math.ceil(totalBits / 8);
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
            return null;
        }

        const collections = this.device.collections;
        if (collections.length > 0) {
            return this.parseReportInfo(collections[0]);
        }
        return null;
    }

    /**
     * 发送输出报告
     * @param {Uint8Array} data - 要发送的数据
     * @param {number} reportId - 报告 ID（默认为 0，自动从描述符获取）
     * @returns {Promise<void>}
     */
    async sendOutputReport(data, reportId = 0) {
        if (!this.device) {
            throw new Error('设备未连接');
        }

        // 检查设备是否仍然打开，如果没有则尝试重新打开
        if (!this.device.opened) {
            try {
                console.log('设备未打开，尝试重新打开...');
                await this.device.open();
            } catch (error) {
                console.error('重新打开设备失败:', error);
                throw new Error('设备连接已断开，请重新连接');
            }
        }

        try {
            // 获取设备的报告信息
            const reportInfo = this.getDeviceReportInfo();
            
            // 如果没有指定 reportId，自动从描述符获取
            if (reportId === 0 && reportInfo) {
                reportId = reportInfo.outputReportId;
                console.log(`自动使用 Output Report ID: ${reportId}`);
            }
            
            // 获取报告大小
            let reportSize = 64; // 默认 HID 报告大小
            if (reportInfo) {
                reportSize = reportInfo.outputReportSize;
            }

            console.log(`报告大小: ${reportSize} 字节, Report ID: ${reportId}`);

            // 确保数据长度符合报告大小要求
            let paddedData = data;
            if (data.length < reportSize) {
                // 数据太短，需要填充到报告大小
                paddedData = new Uint8Array(reportSize);
                paddedData.set(data);
            } else if (data.length > reportSize) {
                // 数据太长，截断到报告大小
                paddedData = data.slice(0, reportSize);
            }

            console.log(`发送报告: reportId=${reportId}, 数据长度=${paddedData.length}, 原始长度=${data.length}`);
            console.log('发送数据:', Array.from(paddedData).map(b => b.toString(16).padStart(2, '0')).join(' '));

            // 使用正确的 reportId 发送
            await this.device.sendReport(reportId, paddedData);
        } catch (error) {
            console.error('发送数据失败:', error);
            console.error('错误名称:', error.name);
            console.error('错误消息:', error.message);

            // 检查是否是真正的权限错误或设备断开
            if (error.name === 'NotAllowedError') {
                // 真正的权限错误，标记为断开
                this.device = null;
                if (this.onDisconnectCallback) {
                    this.onDisconnectCallback();
                }
                throw new Error('设备权限已丢失，请重新连接设备');
            } else if (error.name === 'NotFoundError') {
                // 设备未找到
                this.device = null;
                if (this.onDisconnectCallback) {
                    this.onDisconnectCallback();
                }
                throw new Error('设备未找到，可能已断开连接');
            } else if (error.message && error.message.toLowerCase().includes('transfer')) {
                throw new Error('USB 传输失败，请检查设备连接');
            }
            // 其他错误，保留原始错误信息
            throw new Error(`发送失败: ${error.message || '未知错误'}`);
        }
    }

    /**
     * 获取发送功能报告
     * @param {Uint8Array} data - 要发送的数据
     * @param {number} reportId - 报告 ID（默认为 0，自动从描述符获取）
     * @returns {Promise<DataView>}
     */
    async sendFeatureReport(data, reportId = 0) {
        if (!this.device) {
            throw new Error('设备未连接');
        }

        // 检查设备是否仍然打开，如果没有则尝试重新打开
        if (!this.device.opened) {
            try {
                console.log('设备未打开，尝试重新打开...');
                await this.device.open();
            } catch (error) {
                console.error('重新打开设备失败:', error);
                throw new Error('设备连接已断开，请重新连接');
            }
        }

        try {
            // 获取设备的报告信息
            const reportInfo = this.getDeviceReportInfo();
            
            // 如果没有指定 reportId，自动从描述符获取
            if (reportId === 0 && reportInfo) {
                reportId = reportInfo.featureReportId;
                console.log(`自动使用 Feature Report ID: ${reportId}`);
            }
            
            // 获取报告大小
            let reportSize = 64;
            if (reportInfo) {
                reportSize = reportInfo.featureReportSize;
            }

            // 确保数据长度符合要求
            let paddedData = data;
            if (data.length < reportSize) {
                paddedData = new Uint8Array(reportSize);
                paddedData.set(data);
            } else if (data.length > reportSize) {
                paddedData = data.slice(0, reportSize);
            }

            console.log(`发送功能报告: reportId=${reportId}, 数据长度=${paddedData.length}`);
            console.log('发送数据:', Array.from(paddedData).map(b => b.toString(16).padStart(2, '0')).join(' '));
            await this.device.sendFeatureReport(reportId, paddedData);
        } catch (error) {
            console.error('发送功能报告失败:', error);
            console.error('错误名称:', error.name);
            console.error('错误消息:', error.message);

            // 检查是否是真正的权限错误或设备断开
            if (error.name === 'NotAllowedError') {
                // 真正的权限错误，标记为断开
                this.device = null;
                if (this.onDisconnectCallback) {
                    this.onDisconnectCallback();
                }
                throw new Error('设备权限已丢失，请重新连接设备');
            } else if (error.name === 'NotFoundError') {
                // 设备未找到
                this.device = null;
                if (this.onDisconnectCallback) {
                    this.onDisconnectCallback();
                }
                throw new Error('设备未找到，可能已断开连接');
            }
            // 其他错误，保留原始错误信息
            throw new Error(`发送失败: ${error.message || '未知错误'}`);
        }
    }

    /**
     * 接收功能报告
     * @param {number} reportId - 报告 ID（默认为 0）
     * @returns {Promise<DataView>}
     */
    async receiveFeatureReport(reportId = 0) {
        if (!this.device || !this.device.opened) {
            throw new Error('设备未连接');
        }

        try {
            return await this.device.receiveFeatureReport(reportId);
        } catch (error) {
            console.error('接收功能报告失败:', error);
            throw error;
        }
    }

    /**
     * 检查设备是否已连接
     * @returns {boolean} 是否已连接
     */
    isConnected() {
        return this.device !== null && this.device.opened;
    }

    /**
     * 获取已配对的设备（无需用户选择）
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
}
