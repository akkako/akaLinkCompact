/**
 * 工具函数模块
 */

/**
 * 检查浏览器是否支持 WebHID
 * @returns {boolean} 是否支持
 */
export function isWebHIDSupported() {
    return 'hid' in navigator;
}

/**
 * 将 Uint8Array 转换为十六进制字符串
 * @param {Uint8Array} data - 字节数组
 * @returns {string} 十六进制字符串
 */
export function bytesToHex(data) {
    return Array.from(data)
        .map(b => b.toString(16).padStart(2, '0'))
        .join(' ');
}

/**
 * 将 Uint8Array 转换为 ASCII 字符串
 * @param {Uint8Array} data - 字节数组
 * @returns {string} ASCII 字符串
 */
export function bytesToAscii(data) {
    return Array.from(data)
        .map(b => b >= 32 && b < 127 ? String.fromCharCode(b) : '.')
        .join('');
}

/**
 * 将十六进制字符串转换为 Uint8Array
 * @param {string} hex - 十六进制字符串
 * @returns {Uint8Array} 字节数组
 */
export function hexToBytes(hex) {
    // 移除空格和其他分隔符
    const cleanHex = hex.replace(/\s/g, '');
    const bytes = [];
    for (let i = 0; i < cleanHex.length; i += 2) {
        bytes.push(parseInt(cleanHex.substr(i, 2), 16));
    }
    return new Uint8Array(bytes);
}

/**
 * 格式化时间为本地字符串
 * @param {Date} date - 日期对象
 * @returns {string} 格式化后的时间字符串
 */
export function formatTime(date) {
    return date.toLocaleTimeString('zh-CN', {
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit',
        hour12: false
    });
}

/**
 * 创建 HID 数据包
 * @param {number} reportId - Report ID
 * @param {number} dataLength - 数据长度
 * @param {number} commandType - 命令类型
 * @param {Uint8Array} data - 命令数据（可选）
 * @returns {Uint8Array} 64字节数据包
 */
export function createHIDPacket(reportId, dataLength, commandType, data = null) {
    const packet = new Uint8Array(64);
    packet[0] = reportId;
    packet[1] = dataLength;
    packet[2] = commandType;
    
    if (data) {
        packet.set(data, 3);
    }
    
    return packet;
}

/**
 * 解析配置数据包
 * @param {Uint8Array} data - 接收到的数据
 * @returns {Object|null} 配置对象
 */
export function parseConfigData(data) {
    if (data.length < 6) return null;
    
    // 检查 Report ID 和 Command type
    if (data[0] !== 0x02 || data[2] !== 0x01) return null;
    
    return {
        outputMode: data[3],
        swdMode: data[4],
        v5Mode: data[5],
        clockMode: data[6]
    };
}

/**
 * 解析电压数据
 * @param {Uint8Array} data - 接收到的数据
 * @returns {number|null} 电压值(mV)
 */
export function parseVoltageData(data) {
    if (data.length < 5) return null;
    
    // 检查 Report ID 和 Command type
    if (data[0] !== 0x02 || data[2] !== 0x03) return null;
    
    const lowByte = data[3];
    const highByte = data[4];
    return (highByte << 8) | lowByte;
}

/**
 * 解析型号字符串
 * @param {Uint8Array} data - 接收到的数据
 * @returns {string|null} 型号字符串
 */
export function parseModelString(data) {
    if (data.length < 3) return null;
    
    // 检查 Report ID 和 Command type
    if (data[0] !== 0x02 || data[2] !== 0x10) return null;
    
    // 从第3字节开始读取字符串，直到遇到 \0
    const chars = [];
    for (let i = 3; i < data.length && data[i] !== 0; i++) {
        chars.push(String.fromCharCode(data[i]));
    }
    return chars.join('');
}

/**
 * 解析序列号字符串
 * @param {Uint8Array} data - 接收到的数据
 * @returns {string|null} 序列号字符串
 */
export function parseSNString(data) {
    if (data.length < 3) return null;
    
    // 检查 Report ID 和 Command type
    if (data[0] !== 0x02 || data[2] !== 0x11) return null;
    
    // 从第3字节开始读取字符串，直到遇到 \0
    const chars = [];
    for (let i = 3; i < data.length && data[i] !== 0; i++) {
        chars.push(String.fromCharCode(data[i]));
    }
    return chars.join('');
}

/**
 * 检查响应是否成功
 * @param {Uint8Array} data - 接收到的数据
 * @param {number} expectedCmd - 预期的命令类型
 * @returns {boolean} 是否成功
 */
export function isResponseSuccess(data, expectedCmd) {
    if (data.length < 3) return false;
    return data[0] === 0x02 && data[2] === expectedCmd;
}

/**
 * 延迟函数
 * @param {number} ms - 延迟毫秒数
 * @returns {Promise<void>}
 */
export function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

/**
 * 防抖函数
 * @param {Function} func - 要防抖的函数
 * @param {number} wait - 等待时间（毫秒）
 * @returns {Function} 防抖后的函数
 */
export function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        const later = () => {
            clearTimeout(timeout);
            func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
    };
}
