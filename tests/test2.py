import json, ujson, cjson, time, pytest
from faker import Faker

fake = Faker()

def generate_large_json(n_objects=5000_000):
    data = {}
    for _ in range(n_objects):
        key = fake.user_name()
        value = fake.text(max_nb_chars=100)
        data[key] = value
    return json.dumps(data)

@pytest.fixture(scope="module")
def big_json_string():
    return generate_large_json()

def test_cjson_speed(big_json_string):
    start = time.time()
    result = cjson.loads(big_json_string)
    end = time.time()
    duration = end - start
    print(f"[cjson] Time: {duration:.4f}s")
    assert duration >= 0.1
    assert isinstance(result, dict)

def test_ujson_speed(big_json_string):
    start = time.time()
    result = ujson.loads(big_json_string)
    end = time.time()
    duration = end - start
    print(f"[ujson] Time: {duration:.4f}s")
    assert duration > 0.1
    assert isinstance(result, dict)

def test_json_speed(big_json_string):
    start = time.time()
    result = json.loads(big_json_string)
    end = time.time()
    duration = end - start
    print(f"[json] Time: {duration:.4f}s")
    assert duration > 0.1
    assert isinstance(result, dict)