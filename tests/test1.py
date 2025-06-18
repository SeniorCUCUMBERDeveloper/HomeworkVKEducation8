import json, ujson, cjson, pytest

@pytest.mark.parametrize(
    "text",
    [
        '{"hello": 10, "world": "value"}',
        '{"a":1,"b":2,"c":"3"}',
        '{}',
        '{"ключ":"значение","n":42}',
        '{"emoji":"😀","language":"日本語"}',
        '{"русский":"язык","中文":"语言"}',
        '{    "x":1,\n"y":"2"\t }',
        '{"empty_str":"", "zero":0}',
        '{"neg": -10, "zero": 0, "pos": 999999}',
        '{"big": 1234567890, "small": -987654321}',
        '{"number_as_string": "12345"}',
        '{"long_string": "' + 'x' * 2000 + '"}',
        '{"long_key_' + 'x'*500 + '": "value"}',
        '{' + ','.join(f'"k{i}": {i}' for i in range(100)) + '}'
    ],
)


def test_roundtrip(text):
    py = json.loads(text)
    uj = ujson.loads(text)
    cj = cjson.loads(text)
    assert py == uj == cj
    assert text.replace(" ", "").replace("\n", "").replace("\t", "") == cjson.dumps(cj).replace(" ", "")