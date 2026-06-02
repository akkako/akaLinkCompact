/**
 * 发送管理模块
 * 管理发送条目，处理数据编码和发送
 */

import { parseData, addLengthHeader, formatData } from './utils.js';

/**
 * 发送条目类
 */
export class SendEntry {
    constructor(id, format = 'hex', data = '') {
        this.id = id;
        this.format = format; // 'hex' 或 'ascii'
        this.data = data;
        this.element = null;
    }

    /**
     * 获取字节数据
     * @returns {Uint8Array} 字节数组
     */
    getBytes() {
        try {
            return parseData(this.data, this.format);
        } catch (error) {
            console.error('解析数据失败:', error);
            return new Uint8Array(0);
        }
    }

    /**
     * 获取预览文本
     * @returns {string} 预览字符串
     */
    getPreview() {
        const bytes = this.getBytes();
        if (bytes.length === 0) {
            return '--';
        }
        // 预览同时显示 HEX 和 ASCII
        const hex = formatData(bytes, 'hex');
        const ascii = formatData(bytes, 'ascii');
        return `HEX: ${hex} | ASCII: ${ascii}`;
    }

    /**
     * 验证数据是否有效
     * @returns {boolean} 是否有效
     */
    isValid() {
        if (!this.data || this.data.trim() === '') {
            return false;
        }
        try {
            this.getBytes();
            return true;
        } catch {
            return false;
        }
    }
}

/**
 * 发送管理器类
 */
export class SendManager {
    constructor() {
        this.entries = [];
        this.nextId = 1;
        this.useHeaderLength = false;
        this.headerBytes = 1;
        this.headerEndian = 'little';
        this.sendMode = 'output'; // 'output' 或 'feature'
        this.reportId = 0;
        this.onSendCallback = null;
        this.onEntriesChangeCallback = null;
    }

    /**
     * 设置发送回调
     * @param {Function} callback - 发送数据时的回调 (entry, bytes) => {}
     */
    setOnSend(callback) {
        this.onSendCallback = callback;
    }

    /**
     * 设置条目变化回调
     * @param {Function} callback - 条目变化时的回调
     */
    setOnEntriesChange(callback) {
        this.onEntriesChangeCallback = callback;
    }

    /**
     * 添加新条目
     * @param {string} format - 数据格式 ('hex' 或 'ascii')
     * @param {string} data - 初始数据
     * @returns {SendEntry} 新创建的条目
     */
    addEntry(format = 'hex', data = '') {
        const entry = new SendEntry(this.nextId++, format, data);
        this.entries.push(entry);
        this.notifyEntriesChange();
        return entry;
    }

    /**
     * 删除条目
     * @param {number} id - 条目 ID
     * @returns {boolean} 是否成功删除
     */
    removeEntry(id) {
        const index = this.entries.findIndex(e => e.id === id);
        if (index !== -1) {
            this.entries.splice(index, 1);
            this.renumberEntries();
            this.notifyEntriesChange();
            return true;
        }
        return false;
    }

    /**
     * 重新编号条目
     */
    renumberEntries() {
        this.entries.forEach((entry, index) => {
            entry.id = index + 1;
        });
        this.nextId = this.entries.length + 1;
    }

    /**
     * 更新条目
     * @param {number} id - 条目 ID
     * @param {Object} updates - 要更新的属性
     * @returns {boolean} 是否成功更新
     */
    updateEntry(id, updates) {
        const entry = this.entries.find(e => e.id === id);
        if (entry) {
            if (updates.format !== undefined) {
                // 转换数据格式
                if (entry.format !== updates.format && entry.data) {
                    try {
                        const bytes = entry.getBytes();
                        entry.data = formatData(bytes, updates.format);
                    } catch (error) {
                        // 转换失败时清空数据
                        entry.data = '';
                    }
                }
                entry.format = updates.format;
            }
            if (updates.data !== undefined) {
                entry.data = updates.data;
            }
            return true;
        }
        return false;
    }

    /**
     * 获取所有条目
     * @returns {SendEntry[]} 条目数组
     */
    getEntries() {
        return [...this.entries];
    }

    /**
     * 获取条目数量
     * @returns {number} 条目数量
     */
    getEntryCount() {
        return this.entries.length;
    }

    /**
     * 清空所有条目
     */
    clearEntries() {
        this.entries = [];
        this.nextId = 1;
        this.notifyEntriesChange();
    }

    /**
     * 发送单个条目
     * @param {number} id - 条目 ID
     * @returns {Promise<Object>} 发送结果
     */
    async sendEntry(id) {
        const entry = this.entries.find(e => e.id === id);
        if (!entry) {
            throw new Error('条目不存在');
        }

        if (!entry.isValid()) {
            throw new Error('条目数据无效');
        }

        const rawData = entry.getBytes();
        let dataWithHeader = rawData;
        let headerBytes = 0;

        // 如果需要添加长度头
        if (this.useHeaderLength) {
            dataWithHeader = addLengthHeader(rawData, this.headerBytes, this.headerEndian);
            headerBytes = this.headerBytes;
        }

        if (this.onSendCallback) {
            // 传递原始数据、带长度头的数据、发送模式、Report ID 和长度头字节数
            await this.onSendCallback(entry, rawData, dataWithHeader, this.sendMode, this.reportId, headerBytes);
        }

        return {
            entry: entry,
            rawData: rawData,
            sentData: dataWithHeader,
            sendMode: this.sendMode,
            reportId: this.reportId,
            headerBytes: headerBytes,
            timestamp: new Date()
        };
    }

    /**
     * 发送所有条目
     * @returns {Promise<Object[]>} 发送结果数组
     */
    async sendAllEntries() {
        const results = [];
        for (const entry of this.entries) {
            if (entry.isValid()) {
                try {
                    const result = await this.sendEntry(entry.id);
                    results.push(result);
                } catch (error) {
                    console.error(`发送条目 ${entry.id} 失败:`, error);
                    results.push({
                        entry: entry,
                        error: error.message,
                        timestamp: new Date()
                    });
                }
            }
        }
        return results;
    }

    /**
     * 设置是否使用长度头
     * @param {boolean} use - 是否使用
     */
    setUseHeaderLength(use) {
        this.useHeaderLength = use;
    }

    /**
     * 设置头长度字节数
     * @param {number} bytes - 字节数 (1 或 2)
     */
    setHeaderBytes(bytes) {
        this.headerBytes = bytes;
    }

    /**
     * 设置头字节序
     * @param {string} endian - 'little' 或 'big'
     */
    setHeaderEndian(endian) {
        this.headerEndian = endian;
    }

    /**
     * 设置发送模式
     * @param {string} mode - 'output' 或 'feature'
     */
    setSendMode(mode) {
        this.sendMode = mode;
    }

    /**
     * 设置 Report ID
     * @param {number} reportId - Report ID (0-255)
     */
    setReportId(reportId) {
        this.reportId = parseInt(reportId) || 0;
    }

    /**
     * 获取头配置
     * @returns {Object} 头配置对象
     */
    getHeaderConfig() {
        return {
            useHeaderLength: this.useHeaderLength,
            headerBytes: this.headerBytes,
            headerEndian: this.headerEndian,
            sendMode: this.sendMode,
            reportId: this.reportId
        };
    }

    /**
     * 通知条目变化
     */
    notifyEntriesChange() {
        if (this.onEntriesChangeCallback) {
            this.onEntriesChangeCallback(this.entries);
        }
    }

    /**
     * 导出配置
     * @returns {Object} 配置对象
     */
    exportConfig() {
        return {
            entries: this.entries.map(e => ({
                format: e.format,
                data: e.data
            })),
            headerConfig: this.getHeaderConfig()
        };
    }

    /**
     * 导入配置
     * @param {Object} config - 配置对象
     */
    importConfig(config) {
        this.clearEntries();
        
        if (config.headerConfig) {
            this.setUseHeaderLength(config.headerConfig.useHeaderLength);
            this.setHeaderBytes(config.headerConfig.headerBytes);
            this.setHeaderEndian(config.headerConfig.headerEndian);
        }
        
        if (config.entries && Array.isArray(config.entries)) {
            config.entries.forEach(e => {
                this.addEntry(e.format, e.data);
            });
        }
    }
}
