import json, ujson, cjson, pytest

@pytest.mark.parametrize(
    "text",
    [
        '{"hello": 10, "world": "value"}',
        '{"a":1,"b":2,"c":"3"}',
        '{}',
        '{"ключ":"значение","n":42}'
    ],
)


def test_roundtrip(text):
    py = json.loads(text)
    uj = ujson.loads(text)
    cj = cjson.loads(text)
    assert py == uj == cj
    assert text.replace(" ", "") == cjson.dumps(cj).replace(" ", "")