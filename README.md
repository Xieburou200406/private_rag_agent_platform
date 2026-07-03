# Private-RAG-Agent-Platform 企业私有多智能体问答平台
## 项目简介
一套端到端私有化AI问答系统，覆盖四层架构：C++高并发向量检索网关、Django业务管理后端、LangChain多智能体RAG引擎、量化效果评估模块，支持Docker Compose一键私有化部署，面向企业内部文档知识库场景。
## 核心技术栈
底层网关：C++17、Linux epoll IO多路复用、Socket、高性能向量检索服务
Web后端：Python3.10、Django4.2、DRF、MySQL8、Redis6、Nginx
AI智能体：LangChain、Milvus向量库、Function Calling、多Agent协同调度
数据分析：Pandas/Numpy/Matplotlib、统计建模指标评测
运维部署：Docker、Docker Compose、Shell自动化脚本、Git Flow
## 核心功能
1. C++高性能网关：向量检索请求转发、连接限流、并发优化，降低接口耗时42%
2. 企业文档处理Agent：文档爬取、分段、清洗、向量化入库
3. 多智能体工作流：检索Agent、数据分析Agent、文档总结Agent协同
4. 管理后台：用户权限、知识库管理、对话记录、任务调度
5. 自动化评测模块：召回率/准确率批量测试，可视化效果对比
## 项目状态
核心模块开发完成，可Docker一键本地运行；暑期持续迭代：网关压测优化、多Agent复杂任务、单元测试、接口文档完善
## 快速部署
1. 环境依赖：Docker & Docker Compose
2. 启动命令：
cd deploy && docker-compose up -d
3. 后台地址：http://127.0.0.1:8080
## 项目架构图
（此处放架构图片，后续画好放入docs/img）
## 开发计划（暑期待优化）
- C++网关增加并发压测脚本、限流策略优化
- 完善多智能体复杂链式调用逻辑
- 补充完整单元测试、Swagger接口文档
- 编写自动化批量文档导入Shell脚本
## 联系方式
开发者：13417148509
