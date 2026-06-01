/**
 * UI 控制器模块
 * 负责界面渲染和事件处理
 */

/**
 * UI 控制器类
 */
export class UIController {
    constructor(deviceManager, sendManager, receiveManager) {
        this.deviceManager = deviceManager;
        this.sendManager = sendManager;
        this.receiveManager = receiveManager;
        
        // DOM 元素引用
        this.elements = {};
        
        // 初始化
        this.initElements();
        this.bindEvents();
    }

    /**
     * 初始化 DOM 元素引用
     */
    initElements() {
        // 设备相关
        this.elements.connectBtn = document.getElementById('connectBtn');
        this.elements.disconnectBtn = document.getElementById('disconnectBtn');
        this.elements.connectionStatus = document.getElementById('connectionStatus');
        this.elements.vid = document.getElementById('vid');
        this.elements.pid = document.getElementById('pid');
        this.elements.productName = document.getElementById('productName');

        // 发送配置相关
        this.elements.useHeaderLength = document.getElementById('useHeaderLength');
        this.elements.headerConfig = document.getElementById('headerConfig');
        this.elements.headerLengthBytes = document.getElementById('headerLengthBytes');
        this.elements.headerEndian = document.getElementById('headerEndian');
        this.elements.sendMode = document.getElementById('sendMode');
        this.elements.reportId = document.getElementById('reportId');
        this.elements.sendEntries = document.getElementById('sendEntries');
        this.elements.addEntryBtn = document.getElementById('addEntryBtn');
        this.elements.sendAllBtn = document.getElementById('sendAllBtn');
        this.elements.sendEntryTemplate = document.getElementById('sendEntryTemplate');

        // 接收相关
        this.elements.displayFormat = document.getElementById('displayFormat');
        this.elements.showTimestamp = document.getElementById('showTimestamp');
        this.elements.showSentData = document.getElementById('showSentData');
        this.elements.autoTruncate = document.getElementById('autoTruncate');
        this.elements.receiveOutput = document.getElementById('receiveOutput');
        this.elements.clearReceiveBtn = document.getElementById('clearReceiveBtn');
        this.elements.exportBtn = document.getElementById('exportBtn');
    }

    /**
     * 绑定事件处理器
     */
    bindEvents() {
        // 设备连接/断开
        this.elements.connectBtn.addEventListener('click', () => this.handleConnect());
        this.elements.disconnectBtn.addEventListener('click', () => this.handleDisconnect());

        // 发送配置
        this.elements.useHeaderLength.addEventListener('change', (e) => {
            this.sendManager.setUseHeaderLength(e.target.checked);
            this.elements.headerConfig.style.display = e.target.checked ? 'flex' : 'none';
        });

        this.elements.headerLengthBytes.addEventListener('change', (e) => {
            this.sendManager.setHeaderBytes(parseInt(e.target.value));
        });

        this.elements.headerEndian.addEventListener('change', (e) => {
            this.sendManager.setHeaderEndian(e.target.value);
        });

        this.elements.sendMode.addEventListener('change', (e) => {
            this.sendManager.setSendMode(e.target.value);
        });

        this.elements.reportId.addEventListener('change', (e) => {
            this.sendManager.setReportId(e.target.value);
        });

        // 发送条目管理
        this.elements.addEntryBtn.addEventListener('click', () => this.addSendEntry());
        this.elements.sendAllBtn.addEventListener('click', () => this.handleSendAll());

        // 接收配置
        this.elements.displayFormat.addEventListener('change', (e) => {
            this.receiveManager.setFormat(e.target.value);
        });

        this.elements.showTimestamp.addEventListener('change', (e) => {
            this.receiveManager.setShowTimestamp(e.target.checked);
        });

        this.elements.showSentData.addEventListener('change', (e) => {
            this.receiveManager.setShowSentData(e.target.checked);
        });

        this.elements.autoTruncate.addEventListener('change', (e) => {
            this.receiveManager.setAutoTruncate(e.target.checked);
        });

        // 接收操作
        this.elements.clearReceiveBtn.addEventListener('click', () => {
            this.receiveManager.clear();
        });

        this.elements.exportBtn.addEventListener('click', () => this.handleExport());
    }

    /**
     * 处理连接按钮点击
     */
    async handleConnect() {
        try {
            this.elements.connectBtn.disabled = true;
            await this.deviceManager.requestDevice();
        } catch (error) {
            this.showError(error.message);
            this.elements.connectBtn.disabled = false;
        }
    }

    /**
     * 处理断开连接按钮点击
     */
    async handleDisconnect() {
        await this.deviceManager.disconnect();
    }

    /**
     * 更新设备信息显示
     */
    updateDeviceInfo() {
        const info = this.deviceManager.getDeviceInfo();

        if (info) {
            this.elements.vid.textContent = `0x${info.vendorId.toString(16).toUpperCase().padStart(4, '0')}`;
            this.elements.pid.textContent = `0x${info.productId.toString(16).toUpperCase().padStart(4, '0')}`;
            this.elements.productName.textContent = info.productName || '未知';
        } else {
            this.elements.vid.textContent = '--';
            this.elements.pid.textContent = '--';
            this.elements.productName.textContent = '--';
        }
    }

    /**
     * 更新连接状态显示
     * @param {boolean} connected - 是否已连接
     */
    updateConnectionStatus(connected) {
        if (connected) {
            this.elements.connectionStatus.textContent = '已连接';
            this.elements.connectionStatus.className = 'status-connected';
            this.elements.connectBtn.disabled = true;
            this.elements.disconnectBtn.disabled = false;
            this.elements.sendAllBtn.disabled = false;
        } else {
            this.elements.connectionStatus.textContent = '未连接';
            this.elements.connectionStatus.className = 'status-disconnected';
            this.elements.connectBtn.disabled = false;
            this.elements.disconnectBtn.disabled = true;
            this.elements.sendAllBtn.disabled = true;
        }
        this.updateDeviceInfo();
    }

    /**
     * 添加发送条目
     * @param {string} format - 初始格式
     * @param {string} data - 初始数据
     */
    addSendEntry(format = 'hex', data = '') {
        const entry = this.sendManager.addEntry(format, data);
        this.renderSendEntry(entry);
    }

    /**
     * 渲染发送条目
     * @param {Object} entry - 发送条目对象
     */
    renderSendEntry(entry) {
        const template = this.elements.sendEntryTemplate.content.cloneNode(true);
        const entryDiv = template.querySelector('.send-entry');
        
        // 设置条目索引
        entryDiv.querySelector('.entry-index').textContent = entry.id;
        
        // 设置格式选择
        const formatSelect = entryDiv.querySelector('.entry-format');
        formatSelect.value = entry.format;
        
        // 设置数据输入
        const dataInput = entryDiv.querySelector('.entry-data');
        dataInput.value = entry.data;
        
        // 设置预览
        const previewSpan = entryDiv.querySelector('.preview-text');
        previewSpan.textContent = entry.getPreview();
        
        // 设置发送按钮
        const sendBtn = entryDiv.querySelector('.send-entry-btn');
        sendBtn.disabled = !this.deviceManager.isConnected();
        
        // 绑定事件
        formatSelect.addEventListener('change', (e) => {
            this.sendManager.updateEntry(entry.id, { format: e.target.value });
            previewSpan.textContent = entry.getPreview();
        });

        dataInput.addEventListener('input', (e) => {
            this.sendManager.updateEntry(entry.id, { data: e.target.value });
            previewSpan.textContent = entry.getPreview();
        });

        sendBtn.addEventListener('click', () => this.handleSendEntry(entry.id));

        entryDiv.querySelector('.delete-entry').addEventListener('click', () => {
            this.sendManager.removeEntry(entry.id);
            entryDiv.remove();
            this.renumberSendEntries();
        });

        this.elements.sendEntries.appendChild(entryDiv);
    }

    /**
     * 重新编号发送条目
     */
    renumberSendEntries() {
        const entries = this.elements.sendEntries.querySelectorAll('.send-entry');
        entries.forEach((entryDiv, index) => {
            entryDiv.querySelector('.entry-index').textContent = index + 1;
        });
    }

    /**
     * 更新所有发送按钮状态
     */
    updateSendButtonsState() {
        const connected = this.deviceManager.isConnected();
        const sendButtons = this.elements.sendEntries.querySelectorAll('.send-entry-btn');
        sendButtons.forEach(btn => {
            btn.disabled = !connected;
        });
        this.elements.sendAllBtn.disabled = !connected;
    }

    /**
     * 处理发送单个条目
     * @param {number} entryId - 条目 ID
     */
    async handleSendEntry(entryId) {
        try {
            await this.sendManager.sendEntry(entryId);
        } catch (error) {
            this.showError(`发送失败: ${error.message}`);
        }
    }

    /**
     * 处理发送所有条目
     */
    async handleSendAll() {
        try {
            await this.sendManager.sendAllEntries();
        } catch (error) {
            this.showError(`批量发送失败: ${error.message}`);
        }
    }

    /**
     * 更新接收显示
     */
    updateReceiveDisplay() {
        const html = this.receiveManager.formatAllEntries();
        this.elements.receiveOutput.innerHTML = html;
        this.scrollToBottom();
    }

    /**
     * 滚动接收区到底部
     */
    scrollToBottom() {
        this.elements.receiveOutput.scrollTop = this.elements.receiveOutput.scrollHeight;
    }

    /**
     * 处理导出
     */
    handleExport() {
        const format = this.receiveManager.getDisplayOptions().format;
        const content = this.receiveManager.exportToText();
        
        const blob = new Blob([content], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);
        
        const a = document.createElement('a');
        a.href = url;
        a.download = `hid_data_${format}_${new Date().toISOString().slice(0, 19).replace(/:/g, '-')}.txt`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        
        URL.revokeObjectURL(url);
    }

    /**
     * 显示错误信息
     * @param {string} message - 错误信息
     */
    showError(message) {
        // 创建错误提示元素
        const errorDiv = document.createElement('div');
        errorDiv.className = 'error-message';
        errorDiv.textContent = message;
        
        // 插入到页面顶部
        const container = document.querySelector('.container');
        container.insertBefore(errorDiv, container.firstChild);
        
        // 3秒后自动移除
        setTimeout(() => {
            errorDiv.remove();
        }, 3000);
    }

    /**
     * 显示成功信息
     * @param {string} message - 成功信息
     */
    showSuccess(message) {
        const successDiv = document.createElement('div');
        successDiv.className = 'success-message';
        successDiv.textContent = message;
        
        const container = document.querySelector('.container');
        container.insertBefore(successDiv, container.firstChild);
        
        setTimeout(() => {
            successDiv.remove();
        }, 3000);
    }
}
