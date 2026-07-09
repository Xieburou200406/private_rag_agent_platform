"""离线评测模块入口：从 deploy/.env 读取实验参数，批量拉取数据并输出指标。"""

import os
from pathlib import Path

from dotenv import load_dotenv

# 容器内 /app 挂载源码；本地调试时向上查找 deploy/.env
for env_path in (Path("/app/deploy/.env"), Path(__file__).resolve().parents[1] / "deploy" / ".env"):
    if env_path.exists():
        load_dotenv(env_path)
        break

BATCH_SIZE = int(os.getenv("TEST_BATCH_SIZE", "200"))
TARGET_ACC_UP = float(os.getenv("TARGET_ACC_UP", "28"))
DJANGO_BASE = os.getenv(
    "EVAL_DJANGO_BASE_URL",
    f"http://django_backend:{os.getenv('DJANGO_LISTEN_PORT', '8000')}",
)
GATEWAY_BASE = os.getenv(
    "EVAL_GATEWAY_BASE_URL",
    f"http://cpp_gateway:{os.getenv('GATEWAY_LISTEN_PORT', '8080')}",
)


def main() -> None:
    print("===== evaluation_module 离线评测 =====")
    print(f"batch_size={BATCH_SIZE}, target_acc_up={TARGET_ACC_UP}%")
    print(f"django={DJANGO_BASE}, gateway={GATEWAY_BASE}")
    print("源码就绪后在此实现数据集导出、批量实验与指标可视化。")


if __name__ == "__main__":
    main()
