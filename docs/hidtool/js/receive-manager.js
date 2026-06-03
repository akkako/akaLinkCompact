/**
 * 接收管理模块
 * 管理接收到的数据，支持多种显示模式和同步更新
 */

import { formatData, formatTimestamp, extractLengthAndPayload, bytesToHex } from './utils.js';

/**
 * 数据条目类
 * 存储单条接收或发送的数据
 */
export class DataEntry {
    constructor(data, type, timestamp = new Date(), headerBytes = 0) {
        this.data = data; // Uint8Array
        this.type = type; // 'sent' 或 'received'
        this.timestamp = timestamp;
        this.id = Date.now() + Math.random();
        this.headerBytes = headerBytes; // 长度头字节数（用于发送数据）
    }

    /**
     * 格式化显示
     * @param {Object} options - 显示选项
     * @returns {string} 格式化后的字符串
     */
    format(options = {}) {
        const {
            showTimestamp = true,
            showDirection = true,
            format = 'hex',
            autoTruncate = false,
            headerBytes = 1,
            headerEndian = 'little'
        } = options;

        let result = '';

        // 时间戳
        if (showTimestamp) {
            result += `<span class="timestamp">${formatTimestamp(this.timestamp)}</span>`;
        }

        // 方向标记
        if (showDirection) {
            const direction = this.type === 'sent' ? '→' : '←';
            const directionClass = this.type === 'sent' ? 'sent' : 'received';
            result += `<span class="direction ${directionClass}">${direction}</span>`;
        }

        // 数据内容
        let displayData = this.data;
        
        // 发送数据的特殊处理：
        // - HEX 模式：显示完整数据（包含长度头）
        // - ASCII 模式：忽略长度头，只显示有效数据
        if (this.type === 'sent' && this.headerBytes > 0) {
            if (format === 'ascii') {
                // ASCII 模式：去掉长度头
                displayData = displayData.slice(this.headerBytes);
            }
            // HEX 模式：保持完整数据（包含长度头）
        }
        
        // 如果需要根据长度头截断（仅接收数据）
        if (autoTruncate && this.type === 'received' && displayData.length > headerBytes) {
            const extracted = extractLengthAndPayload(displayData, headerBytes, headerEndian);
            if (extracted.payload.length > 0) {
                displayData = extracted.payload;
            }
        }

        const formattedData = formatData(displayData, format);
        result += `<span class="data">${this.escapeHtml(formattedData)}</span>`;

        return result;
    }

    /**
     * HTML 转义
     * @param {string} text - 原始文本
     * @returns {string} 转义后的文本
     */
    escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }

    /**
     * 获取原始字节数组
     * @returns {Uint8Array} 字节数组
     */
    getBytes() {
        return this.data;
    }

    /**
     * 获取数据长度
     * @returns {number} 数据长度
     */
    getLength() {
        return this.data.length;
    }
}

/**
 * 接收管理器类
 */
export class ReceiveManager {
    constructor() {
        this.entries = []; // 所有数据条目
        this.maxEntries = 1000; // 最大条目数限制
        
        // 显示选项
        this.displayOptions = {
            format: 'hex', // 'hex' 或 'ascii'
            showTimestamp: true,
            showSentData: true,
            autoTruncate: false,
            headerBytes: 1,
            headerEndian: 'little'
        };

        this.onUpdateCallback = null;
    }

    /**
     * 设置更新回调
     * @param {Function} callback - 数据更新时的回调
     */
    setOnUpdate(callback) {
        this.onUpdateCallback = callback;
    }

    /**
     * 添加接收数据
     * @param {Uint8Array} data - 接收到的数据
     * @param {Date} timestamp - 时间戳
     */
    addReceivedData(data, timestamp = new Date()) {
        const entry = new DataEntry(data, 'received', timestamp);
        this.addEntry(entry);
    }

    /**
     * 添加发送数据
     * @param {Uint8Array} data - 发送的数据
     * @param {Date} timestamp - 时间戳
     * @param {number} headerBytes - 长度头字节数
     */
    addSentData(data, timestamp = new Date(), headerBytes = 0) {
        const entry = new DataEntry(data, 'sent', timestamp, headerBytes);
        this.addEntry(entry);
    }

    /**
     * 添加条目
     * @param {DataEntry} entry - 数据条目
     */
    addEntry(entry) {
        this.entries.push(entry);
        
        // 限制最大条目数
        if (this.entries.length > this.maxEntries) {
            this.entries = this.entries.slice(-this.maxEntries);
        }
        
        this.notifyUpdate();
    }

    /**
     * 清空所有数据
     */
    clear() {
        this.entries = [];
        this.notifyUpdate();
    }

    /**
     * 获取所有条目
     * @returns {DataEntry[]} 条目数组
     */
    getEntries() {
        return [...this.entries];
    }

    /**
     * 获取过滤后的条目（根据显示选项）
     * @returns {DataEntry[]} 过滤后的条目数组
     */
    getFilteredEntries() {
        if (this.displayOptions.showSentData) {
            return this.entries;
        } else {
            return this.entries.filter(e => e.type === 'received');
        }
    }

    /**
     * 设置显示格式
     * @param {string} format - 'hex' 或 'ascii'
     */
    setFormat(format) {
        if (this.displayOptions.format !== format) {
            this.displayOptions.format = format;
            this.notifyUpdate();
        }
    }

    /**
     * 设置是否显示时间戳
     * @param {boolean} show - 是否显示
     */
    setShowTimestamp(show) {
        if (this.displayOptions.showTimestamp !== show) {
            this.displayOptions.showTimestamp = show;
            this.notifyUpdate();
        }
    }

    /**
     * 设置是否显示发送数据
     * @param {boolean} show - 是否显示
     */
    setShowSentData(show) {
        if (this.displayOptions.showSentData !== show) {
            this.displayOptions.showSentData = show;
            this.notifyUpdate();
        }
    }

    /**
     * 设置是否自动截断
     * @param {boolean} auto - 是否自动截断
     */
    setAutoTruncate(auto) {
        if (this.displayOptions.autoTruncate !== auto) {
            this.displayOptions.autoTruncate = auto;
            this.notifyUpdate();
        }
    }

    /**
     * 设置头长度字节数
     * @param {number} bytes - 字节数
     */
    setHeaderBytes(bytes) {
        this.displayOptions.headerBytes = bytes;
        if (this.displayOptions.autoTruncate) {
            this.notifyUpdate();
        }
    }

    /**
     * 设置头字节序
     * @param {string} endian - 'little' 或 'big'
     */
    setHeaderEndian(endian) {
        this.displayOptions.headerEndian = endian;
        if (this.displayOptions.autoTruncate) {
            this.notifyUpdate();
        }
    }

    /**
     * 获取显示选项
     * @returns {Object} 显示选项对象
     */
    getDisplayOptions() {
        return { ...this.displayOptions };
    }

    /**
     * 格式化所有条目用于显示
     * @returns {string} HTML 字符串
     */
    formatAllEntries() {
        const filtered = this.getFilteredEntries();
        
        if (filtered.length === 0) {
            return '<div class="empty-state">暂无数据</div>';
        }

        const options = {
            showTimestamp: this.displayOptions.showTimestamp,
            showDirection: true,
            format: this.displayOptions.format,
            autoTruncate: this.displayOptions.autoTruncate,
            headerBytes: this.displayOptions.headerBytes,
            headerEndian: this.displayOptions.headerEndian
        };

        return filtered.map(entry => {
            const formatted = entry.format(options);
            const typeClass = entry.type === 'sent' ? 'sent' : 'received';
            return `<div class="receive-entry ${typeClass}">${formatted}</div>`;
        }).join('');
    }

    /**
     * 导出数据为文本
     * @returns {string} 导出的文本
     */
    exportToText() {
        const options = {
            showTimestamp: this.displayOptions.showTimestamp,
            showDirection: true,
            format: this.displayOptions.format,
            autoTruncate: false // 导出时不截断
        };

        return this.entries.map(entry => {
            const formatted = entry.format(options);
            // 移除 HTML 标签
            return formatted.replace(/<[^>]*>/g, '');
        }).join('\n');
    }

    /**
     * 导出数据为二进制
     * @returns {Uint8Array} 合并后的二进制数据
     */
    exportToBinary() {
        const totalLength = this.entries.reduce((sum, e) => sum + e.getLength(), 0);
        const result = new Uint8Array(totalLength);
        let offset = 0;
        
        for (const entry of this.entries) {
            result.set(entry.getBytes(), offset);
            offset += entry.getLength();
        }
        
        return result;
    }

    /**
     * 导出数据为 JSON
     * @returns {string} JSON 字符串
     */
    exportToJSON() {
        const data = this.entries.map(entry => ({
            type: entry.type,
            timestamp: entry.timestamp.toISOString(),
            data: bytesToHex(entry.getBytes(), ''),
            length: entry.getLength()
        }));
        
        return JSON.stringify(data, null, 2);
    }

    /**
     * 获取统计信息
     * @returns {Object} 统计信息
     */
    getStats() {
        const sent = this.entries.filter(e => e.type === 'sent');
        const received = this.entries.filter(e => e.type === 'received');
        
        return {
            total: this.entries.length,
            sent: sent.length,
            received: received.length,
            sentBytes: sent.reduce((sum, e) => sum + e.getLength(), 0),
            receivedBytes: received.reduce((sum, e) => sum + e.getLength(), 0)
        };
    }

    /**
     * 通知更新
     */
    notifyUpdate() {
        if (this.onUpdateCallback) {
            this.onUpdateCallback();
        }
    }
}
