from django.http import JsonResponse


def health(request):
    return JsonResponse(
        {
            "code": 200,
            "msg": "ok",
            "data": {"service": "django_backend"},
            "request_id": request.headers.get("X-Request-ID"),
        }
    )
