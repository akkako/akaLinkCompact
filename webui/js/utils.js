/**
 * 工具函数模块
 * 提供数据转换、格式化等通用功能
 */

/**
 * 将十六进制字符串转换为字节数组
 * @param {string} hexString - 十六进制字符串（如 "01 02 03" 或 "010203"）
 * @returns {Uint8Array} 字节数组
 */
export function hexToBytes(hexString) {
    // 移除所有空白字符
    const cleaned = hexString.replace(/\s/g, '');
    
    // 检查是否为有效的十六进制字符串
    if (!/^[0-9A-Fa-f]*$/.test(cleaned)) {
        throw new Error('无效的十六进制字符串');
    }
    
    // 长度必须是偶数
    if (cleaned.length % 2 !== 0) {
        throw new Error('十六进制字符串长度必须是偶数');
    }
    
    const bytes = new Uint8Array(cleaned.length / 2);
    for (let i = 0; i < cleaned.length; i += 2) {
        bytes[i / 2] = parseInt(cleaned.substr(i, 2), 16);
    }
    return bytes;
}

/**
 * 将字节数组转换为十六进制字符串
 * @param {Uint8Array} bytes - 字节数组
 * @param {string} separator - 分隔符（默认为空格）
 * @returns {string} 十六进制字符串
 */
export function bytesToHex(bytes, separator = ' ') {
    return Array.from(bytes)
        .map(b => b.toString(16).padStart(2, '0').toUpperCase())
        .join(separator);
}

/**
 * 将ASCII字符串转换为字节数组
 * @param {string} asciiString - ASCII字符串
 * @returns {Uint8Array} 字节数组
 */
export function asciiToBytes(asciiString) {
    const encoder = new TextEncoder();
    return encoder.encode(asciiString);
}

/**
 * 将字节数组转换为ASCII字符串
 * @param {Uint8Array} bytes - 字节数组
 * @returns {string} ASCII字符串（不可打印字符显示为点）
 */
export function bytesToAscii(bytes) {
    return Array.from(bytes)
        .map(b => {
            // 可打印ASCII范围 32-126
            if (b >= 32 && b <= 126) {
                return String.fromCharCode(b);
            }
            // 换行和回车保持原样
            if (b === 10 || b === 13) {
                return String.fromCharCode(b);
            }
            // 其他不可打印字符显示为点
            return '.';
        })
        .join('');
}

/**
 * 根据输入格式解析数据
 * @param {string} data - 输入数据
 * @param {string} format - 格式 ('hex' 或 'ascii')
 * @returns {Uint8Array} 字节数组
 */
export function parseData(data, format) {
    if (!data || data.trim() === '') {
        return new Uint8Array(0);
    }
    
    if (format === 'hex') {
        return hexToBytes(data);
    } else if (format === 'ascii') {
        return asciiToBytes(data);
    }
    throw new Error(`未知的数据格式: ${format}`);
}

/**
 * 格式化数据用于显示
 * @param {Uint8Array} data - 字节数组
 * @param {string} format - 显示格式 ('hex' 或 'ascii')
 * @returns {string} 格式化后的字符串
 */
export function formatData(data, format) {
    if (!data || data.length === 0) {
        return '';
    }
    
    if (format === 'hex') {
        return bytesToHex(data);
    } else if (format === 'ascii') {
        return bytesToAscii(data);
    }
    return '';
}

/**
 * 添加长度头到数据
 * @param {Uint8Array} data - 原始数据
 * @param {number} headerBytes - 头长度字节数 (1 或 2)
 * @param {string} endian - 字节序 ('little' 或 'big')
 * @returns {Uint8Array} 带长度头的数据
 */
export function addLengthHeader(data, headerBytes, endian) {
    const length = data.length;
    const result = new Uint8Array(headerBytes + length);
    
    if (headerBytes === 1) {
        // 1字节长度，最大255
        if (length > 255) {
            throw new Error('数据长度超过255字节，无法使用1字节长度头');
        }
        result[0] = length;
    } else if (headerBytes === 2) {
        // 2字节长度
        if (endian === 'little') {
            result[0] = length & 0xFF;
            result[1] = (length >> 8) & 0xFF;
        } else {
            result[0] = (length >> 8) & 0xFF;
            result[1] = length & 0xFF;
        }
    }
    
    result.set(data, headerBytes);
    return result;
}

/**
 * 从数据中提取长度和有效载荷
 * @param {Uint8Array} data - 带长度头的数据
 * @param {number} headerBytes - 头长度字节数
 * @param {string} endian - 字节序
 * @returns {Object} { length: 声明的长度, payload: 有效载荷, remaining: 剩余数据 }
 */
export function extractLengthAndPayload(data, headerBytes, endian) {
    if (data.length < headerBytes) {
        return { length: 0, payload: new Uint8Array(0), remaining: data };
    }
    
    let declaredLength;
    if (headerBytes === 1) {
        declaredLength = data[0];
    } else if (headerBytes === 2) {
        if (endian === 'little') {
            declaredLength = data[0] | (data[1] << 8);
        } else {
            declaredLength = (data[0] << 8) | data[1];
        }
    }
    
    const totalLength = headerBytes + declaredLength;
    
    if (data.length < totalLength) {
        // 数据不完整
        return { length: declaredLength, payload: new Uint8Array(0), remaining: data };
    }
    
    return {
        length: declaredLength,
        payload: data.slice(headerBytes, totalLength),
        remaining: data.slice(totalLength)
    };
}

/**
 * 格式化时间戳
 * @param {Date} date - 日期对象
 * @returns {string} 格式化的时间字符串 [HH:MM:SS.mmm]
 */
export function formatTimestamp(date = new Date()) {
    const hours = date.getHours().toString().padStart(2, '0');
    const minutes = date.getMinutes().toString().padStart(2, '0');
    const seconds = date.getSeconds().toString().padStart(2, '0');
    const ms = date.getMilliseconds().toString().padStart(3, '0');
    return `[${hours}:${minutes}:${seconds}.${ms}]`;
}

/**
 * 检查浏览器是否支持 WebHID
 * @returns {boolean} 是否支持
 */
export function isWebHIDSupported() {
    return 'hid' in navigator;
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

/**
 * 深拷贝对象
 * @param {Object} obj - 要拷贝的对象
 * @returns {Object} 拷贝后的对象
 */
export function deepClone(obj) {
    return JSON.parse(JSON.stringify(obj));
}
