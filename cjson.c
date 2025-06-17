#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void skip_spaces(const char **p) {
    while (isspace(**p)) (*p)++;
}

static int parse_string(const char **p, char **out, Py_ssize_t *len) {
    const char *s = *p;
    if (*s != '"') return 0;
    s++;
    const char *start = s;
    while (*s && *s != '"') s++;
    if (*s != '"') return 0;
    *len = s - start;
    *out = (char *)start;
    *p   = s + 1;
    return 1;
}

static int parse_number(const char **p, long long *num) {
    char *endptr;
    long long val = strtoll(*p, &endptr, 10);
    if (endptr == *p) return 0;
    *num = val;
    *p   = endptr;
    return 1;
}



static PyObject *cjson_loads(PyObject *self, PyObject *args) {
    const char *json;
    if (!PyArg_ParseTuple(args, "s", &json)) return NULL;

    PyObject *dict = PyDict_New();
    if (!dict) return NULL;

    const char *p = json;
    skip_spaces(&p);
    if (*p != '{') goto error;
    p++;
    skip_spaces(&p);


    if (*p == '}') {
        return dict;
    }

    while (1) {
        char *keystr;
        Py_ssize_t keylen;
        if (!parse_string(&p, &keystr, &keylen)) goto error;
        PyObject *key = PyUnicode_FromStringAndSize(keystr, keylen);
        if (!key) goto error;

        skip_spaces(&p);
        if (*p != ':') { Py_DECREF(key); goto error; }
        p++;
        skip_spaces(&p);


        PyObject *value = NULL;
        if (*p == '"') {
            char *valstr;
            Py_ssize_t vallen;
            if (!parse_string(&p, &valstr, &vallen)) { Py_DECREF(key); goto error; }
            value = PyUnicode_FromStringAndSize(valstr, vallen);
        } else {
            long long num;
            if (!parse_number(&p, &num)) { Py_DECREF(key); goto error; }
            value = PyLong_FromLongLong(num);
        }
        if (!value) { Py_DECREF(key); goto error; }

        if (PyDict_SetItem(dict, key, value) < 0) {
            Py_DECREF(key);
            Py_DECREF(value);
            goto error;
        }
        Py_DECREF(key);
        Py_DECREF(value);

        skip_spaces(&p);
        if (*p == ',') {
            p++;
            skip_spaces(&p);
            continue;
        } else if (*p == '}') {
            p++;
            break;
        } else {
            goto error;
        }
    }

    skip_spaces(&p);
    if (*p != '\0') goto error;

    return dict;

error:
    Py_DECREF(dict);
    PyErr_SetString(PyExc_ValueError, "Invalid or unsupported JSON");
    return NULL;
}


static int append_string(PyObject *buf, const char *s, Py_ssize_t n) {
    return PyList_Append(buf, PyUnicode_FromStringAndSize(s, n));
}

static PyObject *cjson_dumps(PyObject *self, PyObject *args) {
    PyObject *dict;
    if (!PyArg_ParseTuple(args, "O!", &PyDict_Type, &dict)) return NULL;

    PyObject *pieces = PyList_New(0);
    if (!pieces) return NULL;

    if (append_string(pieces, "{", 1) < 0) goto error;

    Py_ssize_t pos = 0;
    PyObject *key, *value;
    int first = 1;
    while (PyDict_Next(dict, &pos, &key, &value)) {
        if (!first) {
            if (append_string(pieces, ", ", 2) < 0) goto error;
        }
        first = 0;

        
        PyObject *key_utf8 = PyUnicode_AsUTF8String(key);
        if (!key_utf8) goto error;
        const char *k = PyBytes_AS_STRING(key_utf8);
        Py_ssize_t kl = PyBytes_GET_SIZE(key_utf8);

        if (append_string(pieces, "\"", 1) < 0) { Py_DECREF(key_utf8); goto error; }
        if (append_string(pieces, k, kl) < 0)    { Py_DECREF(key_utf8); goto error; }
        if (append_string(pieces, "\": ", 3) < 0){ Py_DECREF(key_utf8); goto error; }
        Py_DECREF(key_utf8);


        if (PyUnicode_Check(value)) {
            PyObject *val_utf8 = PyUnicode_AsUTF8String(value);
            if (!val_utf8) goto error;
            const char *v = PyBytes_AS_STRING(val_utf8);
            Py_ssize_t vl = PyBytes_GET_SIZE(val_utf8);

            if (append_string(pieces, "\"", 1) < 0) { Py_DECREF(val_utf8); goto error; }
            if (append_string(pieces, v, vl) < 0)   { Py_DECREF(val_utf8); goto error; }
            if (append_string(pieces, "\"", 1) < 0) { Py_DECREF(val_utf8); goto error; }
            Py_DECREF(val_utf8);

        } else if (PyLong_Check(value)) {
            PyObject *val_str = PyObject_Str(value);
            if (!val_str) goto error;
            const char *v = PyUnicode_AsUTF8(val_str);
            if (!v) { Py_DECREF(val_str); goto error; }
            Py_ssize_t vl = PyUnicode_GET_LENGTH(val_str);
            if (append_string(pieces, v, vl) < 0) { Py_DECREF(val_str); goto error; }
            Py_DECREF(val_str);
        } else {
            PyErr_SetString(PyExc_TypeError, "Unsupported value type for dumps");
            goto error;
        }
    }

    if (append_string(pieces, "}", 1) < 0) goto error;


    PyObject *sep = PyUnicode_FromStringAndSize("", 0);
    PyObject *result = PyUnicode_Join(sep, pieces);
    Py_DECREF(sep);
    Py_DECREF(pieces);
    return result;

error:
    Py_DECREF(pieces);
    return NULL;
}


static PyMethodDef CjsonMethods[] = {
    {"loads", (PyCFunction)cjson_loads, METH_VARARGS, "Parse JSON string"},
    {"dumps", (PyCFunction)cjson_dumps, METH_VARARGS, "Dump dict to JSON string"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef cjsonmodule = {
    PyModuleDef_HEAD_INIT,
    "cjson",
    "Tiny JSON parser/dumper (HW #08)",
    -1,
    CjsonMethods
};

PyMODINIT_FUNC PyInit_cjson(void) {
    return PyModule_Create(&cjsonmodule);
}