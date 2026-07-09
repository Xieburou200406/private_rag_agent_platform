"""Agent 引擎内网入口：/agent/internal/v1/*"""

import argparse
import os

import uvicorn
from dotenv import load_dotenv
from fastapi import FastAPI

load_dotenv()

app = FastAPI(title="RAG Agent Engine", docs_url=None, redoc_url=None)


@app.get("/agent/internal/v1/health")
def health():
    return {"code": 200, "msg": "ok", "data": {"service": "agent_engine"}, "request_id": None}


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=int(os.getenv("AGENT_LISTEN_PORT", "8001")))
    args = parser.parse_args()
    uvicorn.run(app, host=args.host, port=args.port)


if __name__ == "__main__":
    main()
