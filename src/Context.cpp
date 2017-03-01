#include "Context.hpp"

#include "Buffer.hpp"
#include "Texture.hpp"
#include "VertexArray.hpp"
#include "Program.hpp"
#include "Shader.hpp"
#include "Framebuffer.hpp"
#include "EnableFlag.hpp"
#include "InvalidObject.hpp"

#include "BufferFormat.hpp"

PyObject * MGLContext_tp_new(PyTypeObject * type, PyObject * args, PyObject * kwargs) {
	MGLContext * self = (MGLContext *)type->tp_alloc(type, 0);

	#ifdef MGL_VERBOSE
	printf("MGLContext_tp_new %p\n", self);
	#endif

	if (self) {
		self->rc_handle = 0;
		self->dc_handle = 0;
	}

	return (PyObject *)self;
}

void MGLContext_tp_dealloc(MGLContext * self) {

	#ifdef MGL_VERBOSE
	printf("MGLContext_tp_dealloc %p\n", self);
	#endif

	Py_TYPE(self)->tp_free((PyObject*)self);
}

int MGLContext_tp_init(MGLContext * self, PyObject * args, PyObject * kwargs) {
	return -1;
}

PyObject * MGLContext_tp_str(MGLContext * self) {
	return PyUnicode_FromFormat("<ModernGL.Context>");
}

// PyObject * MGLContext_make_current(MGLContext * self) {
// 	if (!wglMakeCurrent(self->dc_handle, self->rc_handle)) {
// 		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
// 		return 0;
// 	}

// 	Py_RETURN_NONE;
// }

const char * MGLContext_make_current_doc = R"(
)";

PyObject * MGLContext_clear(MGLContext * self, PyObject * args, PyObject * kwargs) {
	static const char * kwlist[] = {"r", "g", "b", "a", 0};

	int r = 0;
	int g = 0;
	int b = 0;
	int a = 0;

	int args_ok = PyArg_ParseTupleAndKeywords(
		args,
		kwargs,
		"|BBBB",
		(char **)kwlist,
		&r,
		&g,
		&b,
		&a
	);

	if (!args_ok) {
		// PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	const float c = 1.0f / 255.0f;

	GLMethods & gl = self->gl;
	gl.ClearColor(r * c, g * c, b * c, a * c);
	gl.Clear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	Py_RETURN_NONE;
}

const char * MGLContext_clear_doc = R"(
	clear(r = 0, g = 0, b = 0, a = 0)

	Clear the current framebuffer.

	Args:
		r, g, b (int): clear color.
		a (int): alpha.

	Returns:
		None
)";

PyObject * MGLContext_enable(MGLContext * self, PyObject * args, PyObject * kwargs) {
	static const char * kwlist[] = {"flags", 0};

	MGLEnableFlag * flags;

	int args_ok = PyArg_ParseTupleAndKeywords(
		args,
		kwargs,
		"O!",
		(char **)kwlist,
		&MGLEnableFlag_Type,
		&flags
	);

	if (!args_ok) {
		// PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	self->gl.Enable(flags->flag);

	Py_RETURN_NONE;
}

const char * MGLContext_enable_doc = R"(
	enable(flags)

	Enable flags.

	Args:
		flags (~ModernGL.EnableFlag): flags to enable.

	Returns:
		None
)";

PyObject * MGLContext_disable(MGLContext * self, PyObject * args, PyObject * kwargs) {
	static const char * kwlist[] = {"flags", 0};

	MGLEnableFlag * flags;

	int args_ok = PyArg_ParseTupleAndKeywords(
		args,
		kwargs,
		"O!",
		(char **)kwlist,
		&MGLEnableFlag_Type,
		&flags
	);

	if (!args_ok) {
		// PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	self->gl.Disable(flags->flag);

	Py_RETURN_NONE;
}

const char * MGLContext_disable_doc = R"(
	disable(flags)

	Disable flags.

	Args:
		flags (~ModernGL.EnableFlag): flags to disable.

	Returns:
		None
)";

PyObject * MGLContext_finish(MGLContext * self) {
	self->gl.Finish();
	Py_RETURN_NONE;
}

const char * MGLContext_finish_doc = R"(
	finish()

	Disable flags.

	Args:
		flags (~ModernGL.EnableFlag): flags to disable.

	Returns:
		None
)";

PyObject * MGLContext_info(MGLContext * self) {
	return 0;
}

const char * MGLContext_info_doc = R"(
)";

PyObject * MGLContext_copy_buffer(MGLContext * self, PyObject * args, PyObject * kwargs) {
	static const char * kwlist[] = {"dst", "src", "size", "read_offset", "write_offset", 0};

	MGLBuffer * dst;
	MGLBuffer * src;

	int size = -1;
	int read_offset = 0;
	int write_offset = 0;

	int args_ok = PyArg_ParseTupleAndKeywords(
		args,
		kwargs,
		"O!O!|I$II",
		(char **)kwlist,
		&MGLBuffer_Type,
		&dst,
		&MGLBuffer_Type,
		&src,
		&size,
		&read_offset,
		&write_offset
	);

	if (!args_ok) {
		// PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	if (size < 0) {
		size = src->size - read_offset;
	}

	// TODO: error checking

	GLMethods & gl = self->gl;

	gl.BindBuffer(GL_COPY_READ_BUFFER, src->obj);
	gl.BindBuffer(GL_COPY_WRITE_BUFFER, dst->obj);
	gl.CopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, read_offset, write_offset, size);

	Py_RETURN_NONE;
}

const char * MGLContext_copy_buffer_doc = R"(
	copy_buffer(dst, src)

	Copy buffer content.
)";

MGLBuffer * MGLContext_Buffer(MGLContext * self, PyObject * args, PyObject * kwargs) {
	static const char * kwlist[] = {"data", "reserve", "dynamic", 0};

	PyObject * data = Py_None;
	int reserve = 0;
	int dynamic = false;

	int args_ok = PyArg_ParseTupleAndKeywords(
		args,
		kwargs,
		"|O$Ip",
		(char **)kwlist,
		&data,
		&reserve,
		&dynamic
	);

	if (!args_ok) {
		// PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	if ((data != Py_None && reserve) || (data == Py_None && !reserve)) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	Py_buffer buffer_view;

	if (data != Py_None) {
		int get_buffer = PyObject_GetBuffer(data, &buffer_view, PyBUF_SIMPLE);
		if (get_buffer < 0) {
			PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
			return 0;
		}
	} else {
		buffer_view.len = reserve;
		buffer_view.buf = 0;
	}

	if (!buffer_view.len) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	MGLBuffer * buffer = MGLBuffer_New();

	buffer->size = buffer_view.len;
	buffer->dynamic = dynamic ? true : false;

	buffer->obj = 0;

	GLMethods & gl = self->gl;

	buffer->obj = 0;
	gl.GenBuffers(1, (GLuint *)&buffer->obj);
	gl.BindBuffer(GL_ARRAY_BUFFER, buffer->obj);
	gl.BufferData(GL_ARRAY_BUFFER, buffer->size, buffer_view.buf, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

	Py_INCREF(self);
	buffer->context = self;

	if (data != Py_None) {
		PyBuffer_Release(&buffer_view);
	}

	Py_INCREF(buffer);
	return buffer;
}

const char * MGLContext_Buffer_doc = R"(
	Buffer(...)

	Create a Buffer.
)";

MGLTexture * MGLContext_Texture(MGLContext * self, PyObject * args, PyObject * kwargs) {
	static const char * kwlist[] = {"size", "components", "data", "floats", 0};

	int width;
	int height;

	int components = 3;

	PyObject * data = Py_None;

	int floats = false;

	int args_ok = PyArg_ParseTupleAndKeywords(
		args,
		kwargs,
		"(II)|IO$p",
		(char **)kwlist,
		&width,
		&height,
		&components,
		&data,
		&floats
	);

	if (!args_ok) {
		// PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	if (components < 1 || components > 4) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	int expected_size = floats ? (width * height * components * 4) : (height * ((width * components + 3) & ~3));

	Py_buffer buffer_view;

	if (data != Py_None) {
		PyObject_GetBuffer(data, &buffer_view, PyBUF_SIMPLE);
	} else {
		buffer_view.len = expected_size;
		buffer_view.buf = 0;
	}

	if (buffer_view.len != expected_size) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		if (data != Py_None) {
			PyBuffer_Release(&buffer_view);
		}
		return 0;
	}

	const int formats[] = {0, GL_RED, GL_RG, GL_RGB, GL_RGBA};

	int pixel_type = floats ? GL_FLOAT : GL_UNSIGNED_BYTE;
	int format = formats[components];

	GLMethods & gl = self->gl;

	gl.ActiveTexture(GL_TEXTURE0 + self->default_texture_unit);

	MGLTexture * texture = MGLTexture_New();

	texture->obj = 0;
	gl.GenTextures(1, (GLuint *)&texture->obj);
	gl.BindTexture(GL_TEXTURE_2D, texture->obj);
	gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl.TexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, pixel_type, buffer_view.buf);

	if (data != Py_None) {
		PyBuffer_Release(&buffer_view);
	}

	texture->width = width;
	texture->height = height;
	texture->components = components;
	texture->floats = floats ? true : false;
	texture->depth = false;

	Py_INCREF(self);
	texture->context = self;

	Py_INCREF(texture);
	return texture;
}

const char * MGLContext_Texture_doc = R"(
	Texture(...)

	Create a Texture.
)";

MGLTexture * MGLContext_DepthTexture(MGLContext * self, PyObject * args, PyObject * kwargs) {
	static const char * kwlist[] = {"size", "data", 0};

	int width;
	int height;

	PyObject * data = Py_None;

	int args_ok = PyArg_ParseTupleAndKeywords(
		args,
		kwargs,
		"(II)|O",
		(char **)kwlist,
		&width,
		&height,
		&data
	);

	if (!args_ok) {
		// PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	int expected_size = width * height * 4;

	Py_buffer buffer_view;

	if (data != Py_None) {
		PyObject_GetBuffer(data, &buffer_view, PyBUF_SIMPLE);
	} else {
		buffer_view.len = expected_size;
		buffer_view.buf = 0;
	}

	if (buffer_view.len != expected_size) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		if (data != Py_None) {
			PyBuffer_Release(&buffer_view);
		}
		return 0;
	}

	int pixel_type = GL_FLOAT;
	int format = GL_RED;

	GLMethods & gl = self->gl;

	gl.ActiveTexture(GL_TEXTURE0 + self->default_texture_unit);

	MGLTexture * texture = MGLTexture_New();

	texture->obj = 0;
	gl.GenTextures(1, (GLuint *)&texture->obj);
	gl.BindTexture(GL_TEXTURE_2D, texture->obj);
	gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl.TexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, pixel_type, buffer_view.buf);

	if (data != Py_None) {
		PyBuffer_Release(&buffer_view);
	}

	texture->width = width;
	texture->height = height;
	texture->components = 1;
	texture->floats = true;
	texture->depth = true;

	Py_INCREF(self);
	texture->context = self;

	Py_INCREF(texture);
	return texture;
}

const char * MGLContext_DepthTexture_doc = R"(
	DepthTexture(...)

	Create a DepthTexture.
)";

MGLVertexArray * MGLContext_VertexArray(MGLContext * self, PyObject * args, PyObject * kwargs) {
	static const char * kwlist[] = {"program", "content", "index_buffer", "skip_errors", 0};

	MGLProgram * program;
	PyObject * content;
	MGLBuffer * index_buffer = (MGLBuffer *)Py_None;
	int skip_errors = false;

	int args_ok = PyArg_ParseTupleAndKeywords(
		args,
		kwargs,
		"O!O!|O$p",
		(char **)kwlist,
		&MGLProgram_Type,
		&program,
		&PyList_Type,
		&content,
		&index_buffer,
		&skip_errors
	);

	if (!args_ok) {
		// PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	GLMethods & gl = self->gl;

	int content_len = PyList_GET_SIZE(content);

	for (int i = 0; i < content_len; ++i) {
		PyObject * tuple = PyList_GET_ITEM(content, i);

		if (Py_TYPE(tuple) != &PyTuple_Type) {
			PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
			return 0;
		}

		int size = PyTuple_GET_SIZE(tuple);

		if (size != 3) {
			PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
			return 0;
		}

		PyObject * buffer = PyTuple_GET_ITEM(tuple, 0);
		PyObject * format = PyTuple_GET_ITEM(tuple, 1);
		PyObject * attributes = PyTuple_GET_ITEM(tuple, 2);

		if (Py_TYPE(buffer) != &MGLBuffer_Type) {
			PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
			return 0;
		}

		if (Py_TYPE(format) != &PyUnicode_Type) {
			PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
			return 0;
		}

		if (Py_TYPE(attributes) != &PyList_Type) {
			PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
			return 0;
		}

		FormatIterator it = FormatIterator(PyUnicode_AsUTF8(format));
		FormatInfo format_info = it.info();

		if (!format_info.valid) {
			PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
			return 0;
		}

		int attributes_len = PyList_GET_SIZE(attributes);

		for (int j = 0; j < attributes_len; ++j) {
			PyObject * attribute = PyList_GET_ITEM(attributes, j);

			if (Py_TYPE(attribute) != &PyUnicode_Type) {
				PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
				return 0;
			}

			if (!skip_errors) {
				int location = gl.GetAttribLocation(program->obj, PyUnicode_AsUTF8(attribute));

				if (location < 0) {
					PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
					return 0;
				}
			}
		}
	}

	if (index_buffer != (MGLBuffer *)Py_None && Py_TYPE(index_buffer) != &MGLBuffer_Type) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	PyObject * vertex_array_content = PyTuple_New(content_len);

	for (int i = 0; i < content_len; ++i) {
		PyObject * tuple = PyList_GET_ITEM(content, i);

		PyObject * buffer = PyTuple_GET_ITEM(tuple, 0);
		PyObject * format = PyTuple_GET_ITEM(tuple, 1);
		PyObject * attributes = PyTuple_GET_ITEM(tuple, 2);

		PyObject * new_tuple = PyTuple_Pack(3, buffer, format, PyList_AsTuple(attributes));
		PyTuple_SET_ITEM(vertex_array_content, i, new_tuple);
	}

	MGLVertexArray * array = MGLVertexArray_New();

	Py_INCREF(program);
	array->program = program;

	array->obj = 0;
	gl.GenVertexArrays(1, (GLuint *)&array->obj);
	gl.BindVertexArray(array->obj);

	Py_INCREF(index_buffer);
	array->index_buffer = index_buffer;

	if (index_buffer != (MGLBuffer *)Py_None) {
		array->num_vertices = index_buffer->size / 4;
		gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer->obj);
	} else {
		array->num_vertices = -1;
	}

	// printf("array->num_vertices: %d\n", array->num_vertices);

	for (int i = 0; i < content_len; ++i) {
		PyObject * tuple = PyTuple_GET_ITEM(vertex_array_content, i);

		MGLBuffer * buffer = (MGLBuffer *)PyTuple_GET_ITEM(tuple, 0);
		char * format = PyUnicode_AsUTF8(PyTuple_GET_ITEM(tuple, 1));
		PyObject * attributes = PyTuple_GET_ITEM(tuple, 2);

		FormatIterator it = FormatIterator(format);
		FormatInfo format_info = it.info();

		if (!format_info.valid) {
			PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
			return 0;
		}

		int buf_vertices = buffer->size / format_info.size;

		// printf("buf_vertices: %d\n", buf_vertices);

		if (!format_info.per_instance && array->index_buffer == (MGLBuffer *)Py_None && (!i || array->num_vertices > buf_vertices)) {
			array->num_vertices = buf_vertices;
		}

		// printf("array->num_vertices: %d\n", array->num_vertices);

		gl.BindBuffer(GL_ARRAY_BUFFER, buffer->obj);

		char * ptr = 0;

		int attributes_len = PyTuple_GET_SIZE(attributes);

		for (int j = 0; j < attributes_len; ++j) {
			char * attribute = PyUnicode_AsUTF8(PyTuple_GET_ITEM(attributes, j));
			int location = gl.GetAttribLocation(program->obj, attribute);

			FormatNode * node = it.next();

			if (location >= 0) {
				if (node->type == GL_INT) {
					gl.VertexAttribIPointer(location, node->count, node->type, format_info.size, ptr);
				} else {
					gl.VertexAttribPointer(location, node->count, node->type, false, format_info.size, ptr);
				}

				if (format_info.per_instance) {
					gl.VertexAttribDivisor(location, 1);
				}

				gl.EnableVertexAttribArray(location);
			}

			ptr += node->count * node->size;
		}
	}

	Py_INCREF(self);
	array->context = self;

	Py_INCREF(array);
	return array;
}

const char * MGLContext_VertexArray_doc = R"(
	VertexArray(...)

	Create a VertexArray.
)";

MGLVertexArray * MGLContext_SimpleVertexArray(MGLContext * self, PyObject * args, PyObject * kwargs) {
	static const char * kwlist[] = {"program", "buffer", "format", "attributes", "index_buffer", "skip_errors", 0};

	PyObject * program;
	PyObject * buffer;
	PyObject * format;
	PyObject * attributes;
	PyObject * index_buffer = Py_None;
	PyObject * skip_errors = Py_False;

	int args_ok = PyArg_ParseTupleAndKeywords(
		args,
		kwargs,
		"O!O!O!O!|O$O!",
		(char **)kwlist,
		&MGLProgram_Type,
		&program,
		&MGLBuffer_Type,
		&buffer,
		&PyUnicode_Type,
		&format,
		&PyList_Type,
		&attributes,
		&index_buffer,
		&PyBool_Type,
		&skip_errors
	);

	if (!args_ok) {
		// PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	Py_INCREF(program);
	Py_INCREF(buffer);
	Py_INCREF(format);
	Py_INCREF(attributes);
	Py_INCREF(index_buffer);

	PyObject * new_tuple = PyTuple_New(3);
	PyTuple_SET_ITEM(new_tuple, 0, buffer);
	PyTuple_SET_ITEM(new_tuple, 1, format);
	PyTuple_SET_ITEM(new_tuple, 2, attributes);

	PyObject * content = PyList_New(1);
	PyList_SET_ITEM(content, 0, new_tuple);

	PyObject * new_args = PyTuple_New(3);
	PyTuple_SET_ITEM(new_args, 0, program);
	PyTuple_SET_ITEM(new_args, 1, content);
	PyTuple_SET_ITEM(new_args, 2, index_buffer);

	PyObject * new_kwargs = PyDict_New();
	PyDict_SetItemString(new_kwargs, "skip_errors", skip_errors);

	MGLVertexArray * result = MGLContext_VertexArray(self, new_args, new_kwargs);

	Py_DECREF(new_args);
	Py_DECREF(new_kwargs);

	return result;
}

const char * MGLContext_SimpleVertexArray_doc = R"(
	SimpleVertexArray(...)

	Create a SimpleVertexArray.
)";

MGLProgram * MGLContext_Program(MGLContext * self, PyObject * args, PyObject * kwargs) {
	static const char * kwlist[] = {"shaders", "varyings", 0};

	PyObject * shaders;
	PyObject * varyings = Py_None;

	int args_ok = PyArg_ParseTupleAndKeywords(
		args,
		kwargs,
		"O|O",
		(char **)kwlist,
		&shaders,
		&varyings
	);

	if (!args_ok) {
		// PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	if (varyings != Py_None) {
		if (Py_TYPE(varyings) != &PyList_Type) {
			PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
			return 0;
		}

		int num_varyings = PyList_GET_SIZE(varyings);

		for (int i = 0; i < num_varyings; ++i) {
			PyObject * item = PyList_GET_ITEM(varyings, i);
			if (Py_TYPE(item) != &PyUnicode_Type) {
				PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
				return 0;
			}
		}
	}

	PyObject * program_shaders = 0;

	if (Py_TYPE(shaders) == &PyList_Type) {

		int num_shaders = PyList_GET_SIZE(shaders);

		for (int i = 0; i < num_shaders; ++i) {
			PyObject * item = PyList_GET_ITEM(shaders, i);
			if (Py_TYPE(item) != &MGLShader_Type) {
				PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
				return 0;
			}

			// check duplicate types
		}

		program_shaders = PyList_AsTuple(shaders);

	} else if (Py_TYPE(shaders) == &MGLShader_Type) {

		Py_INCREF(shaders);
		program_shaders = PyTuple_Pack(1, shaders);

	} else {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	GLMethods & gl = self->gl;

	MGLProgram * program = MGLProgram_New();

	// printf("Program: %d\n", Py_REFCNT(program));

	program->shaders = program_shaders;

	Py_INCREF(self);
	program->context = self;

	MGLProgram_Compile(program, varyings);

	if (PyErr_Occurred()) {
		Py_DECREF(program);
		return 0;
	}

	Py_INCREF(program);
	return program;
}

const char * MGLContext_Program_doc = R"(
	Program(...)

	Create a Program.
)";

template <int ShaderType>
MGLShader * MGLContext_Shader(MGLContext * self, PyObject * args, PyObject * kwargs) {
	static const char * kwlist[] = {"source", 0};

	PyObject * source;

	int args_ok = PyArg_ParseTupleAndKeywords(
		args,
		kwargs,
		"O",
		(char **)kwlist,
		&source
	);

	if (!args_ok) {
		// PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	if (!PyUnicode_Check(source)) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	MGLShader * shader = MGLShader_New();

	shader->shader_type = ShaderType;

	Py_INCREF(source);
	shader->source = source;

	Py_INCREF(self);
	shader->context = self;

	MGLShader_Compile(shader);

	if (PyErr_Occurred()) {
		Py_DECREF(shader);
		return 0;
	}

	// printf("%p %d\n", shader, Py_REFCNT(shader));

	Py_INCREF(shader);
	return shader;
}

const char * MGLContext_Shader_doc = R"(
	Shader(...)

	Create a Shader.
)";

MGLFramebuffer * MGLContext_Framebuffer(MGLContext * self, PyObject * args, PyObject * kwargs) {
	static const char * kwlist[] = {"attachments", 0};

	PyObject * attachments;

	int args_ok = PyArg_ParseTupleAndKeywords(
		args,
		kwargs,
		"O!",
		(char **)kwlist,
		&PyList_Type,
		&attachments
	);

	if (!args_ok) {
		// PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	int attachments_len = PyList_GET_SIZE(attachments);

	bool has_depth_attachment = false;

	for (int i = 0; i < attachments_len; ++i) {
		PyObject * item = PyList_GET_ITEM(attachments, i);
		if (Py_TYPE(item) == &MGLTexture_Type) {
			MGLTexture * texture = (MGLTexture *)item;
			if (texture->depth) {
				if (has_depth_attachment) {
					PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
					return 0;
				}
				has_depth_attachment = true;
			}
		// } else if (Py_TYPE(item) == &MGLTexture_Type) {
		// 	MGLTexture * texture = (MGLTexture *)item;
		// 	if (texture->depth) {
		// 		if (has_depth_attachment) {
		// 			PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		// 			return 0;
		// 		}
		// 		has_depth_attachment = true;
		// 	}
		} else {
			PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
			return 0;
		}
	}

	PyObject * color_attachments = PyTuple_New(attachments_len - has_depth_attachment ? 1 : 0);
	PyObject * depth_attachment = Py_None;

	int color_attachments_len = 0;

	GLMethods & gl = self->gl;

	int draw_framebuffer = 0;
	gl.GetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&draw_framebuffer);

	MGLFramebuffer * framebuffer = MGLFramebuffer_New();

	framebuffer->obj = 0;
	gl.GenFramebuffers(1, (GLuint *)&framebuffer->obj);
	gl.BindFramebuffer(GL_FRAMEBUFFER, framebuffer->obj);

	for (int i = 0; i < attachments_len; ++i) {
		PyObject * item = PyList_GET_ITEM(attachments, i);
		if (Py_TYPE(item) == &MGLTexture_Type) {
			MGLTexture * texture = (MGLTexture *)item;
			if (texture->depth) {
				gl.FramebufferTexture2D(
					GL_FRAMEBUFFER,
					GL_DEPTH_ATTACHMENT,
					GL_TEXTURE_2D,
					texture->obj,
					0
				);

				Py_INCREF(item);
				depth_attachment = item;
			} else {

				gl.FramebufferTexture2D(
					GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + i,
					GL_TEXTURE_2D,
					texture->obj,
					0
				);

				PyTuple_SET_ITEM(color_attachments, color_attachments_len, item);
				color_attachments_len += 1;
			}
		// } else if (Py_TYPE(item) == &MGLTexture_Type) {
		}
	}

	gl.BindFramebuffer(GL_FRAMEBUFFER, draw_framebuffer);

	int status = gl.CheckFramebufferStatus(framebuffer->obj);

	if (status != GL_FRAMEBUFFER_COMPLETE) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return 0;
	}

	framebuffer->color_attachments = color_attachments;
	framebuffer->depth_attachment = depth_attachment;

	Py_INCREF(self);
	framebuffer->context = self;

	Py_INCREF(framebuffer);
	return framebuffer;
}

const char * MGLContext_Framebuffer_doc = R"(
	Framebuffer(...)

	Create a Framebuffer.
)";

PyMethodDef MGLContext_tp_methods[] = {
	// {"make_current", (PyCFunction)MGLContext_make_current, METH_NOARGS, MGLContext_make_current_doc},

	{"clear", (PyCFunction)MGLContext_clear, METH_VARARGS | METH_KEYWORDS, MGLContext_clear_doc},
	{"enable", (PyCFunction)MGLContext_enable, METH_VARARGS | METH_KEYWORDS, MGLContext_enable_doc},
	{"disable", (PyCFunction)MGLContext_disable, METH_VARARGS | METH_KEYWORDS, MGLContext_disable_doc},
	{"finish", (PyCFunction)MGLContext_finish, METH_NOARGS, MGLContext_finish_doc},
	{"copy_buffer", (PyCFunction)MGLContext_copy_buffer, METH_VARARGS | METH_KEYWORDS, MGLContext_copy_buffer_doc},

	{"Buffer", (PyCFunction)MGLContext_Buffer, METH_VARARGS | METH_KEYWORDS, MGLContext_Buffer_doc},
	{"Texture", (PyCFunction)MGLContext_Texture, METH_VARARGS | METH_KEYWORDS, MGLContext_Texture_doc},
	{"DepthTexture", (PyCFunction)MGLContext_DepthTexture, METH_VARARGS | METH_KEYWORDS, MGLContext_DepthTexture_doc},
	{"VertexArray", (PyCFunction)MGLContext_VertexArray, METH_VARARGS | METH_KEYWORDS, MGLContext_VertexArray_doc},
	{"SimpleVertexArray", (PyCFunction)MGLContext_SimpleVertexArray, METH_VARARGS | METH_KEYWORDS, MGLContext_SimpleVertexArray_doc},
	{"Program", (PyCFunction)MGLContext_Program, METH_VARARGS | METH_KEYWORDS, MGLContext_Program_doc},
	{"VertexShader", (PyCFunction)MGLContext_Shader<GL_VERTEX_SHADER>, METH_VARARGS | METH_KEYWORDS, MGLContext_Shader_doc},
	{"FragmentShader", (PyCFunction)MGLContext_Shader<GL_FRAGMENT_SHADER>, METH_VARARGS | METH_KEYWORDS, MGLContext_Shader_doc},
	{"GeometryShader", (PyCFunction)MGLContext_Shader<GL_GEOMETRY_SHADER>, METH_VARARGS | METH_KEYWORDS, MGLContext_Shader_doc},
	{"TessEvaluationShader", (PyCFunction)MGLContext_Shader<GL_TESS_EVALUATION_SHADER>, METH_VARARGS | METH_KEYWORDS, MGLContext_Shader_doc},
	{"TessControlShader", (PyCFunction)MGLContext_Shader<GL_TESS_CONTROL_SHADER>, METH_VARARGS | METH_KEYWORDS, MGLContext_Shader_doc},
	{"Framebuffer", (PyCFunction)MGLContext_Framebuffer, METH_VARARGS | METH_KEYWORDS, MGLContext_Framebuffer_doc},

	{0},
};

int MGLContext_set_line_width(MGLContext * self, PyObject * value) {
	float line_width = (float)PyFloat_AsDouble(value);

	if (PyErr_Occurred()) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

	self->gl.LineWidth(line_width);

	return 0;
}

char MGLContext_line_width_doc[] = R"(
	line_width

	Set the default line width.
)";

int MGLContext_set_point_size(MGLContext * self, PyObject * value) {
	float point_size = (float)PyFloat_AsDouble(value);

	if (PyErr_Occurred()) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

	self->gl.PointSize(point_size);

	return 0;
}

char MGLContext_point_size_doc[] = R"(
	point_size

	Set the default point size.
)";

PyObject * MGLContext_get_viewport(MGLContext * self) {
	int viewport[4] = {};

	self->gl.GetIntegerv(GL_VIEWPORT, viewport);

	PyObject * x = PyLong_FromLong(viewport[0]);
	PyObject * y = PyLong_FromLong(viewport[1]);
	PyObject * width = PyLong_FromLong(viewport[2]);
	PyObject * height = PyLong_FromLong(viewport[3]);

	return PyTuple_Pack(4, x, y, width, height);
}

int MGLContext_set_viewport(MGLContext * self, PyObject * value) {
	int size = PyTuple_Size(value);

	if (PyErr_Occurred()) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

	if (size != 4) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

	int x = PyLong_AsLong(PyTuple_GET_ITEM(value, 0));
	int y = PyLong_AsLong(PyTuple_GET_ITEM(value, 1));
	int width = PyLong_AsLong(PyTuple_GET_ITEM(value, 2));
	int height = PyLong_AsLong(PyTuple_GET_ITEM(value, 3));

	if (PyErr_Occurred()) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

	self->gl.Viewport(x, y, width, height);

	return 0;
}

char MGLContext_viewport_doc[] = R"(
	viewport

	The viewport.
)";

PyObject * MGLContext_get_default_texture_unit(MGLContext * self) {
	return PyLong_FromLong(self->default_texture_unit);
}

int MGLContext_set_default_texture_unit(MGLContext * self, PyObject * value) {
	int default_texture_unit = PyLong_AsLong(value);

	if (PyErr_Occurred()) {
		PyErr_Format(PyExc_Exception, "Unknown error in %s (%s:%d)", __FUNCTION__, __FILE__, __LINE__);
		return -1;
	}

	self->default_texture_unit = default_texture_unit;

	return 0;
}

char MGLContext_default_texture_unit_doc[] = R"(
	default_texture_unit

	The default texture unit.
)";

PyObject * MGLContext_get_max_texture_units(MGLContext * self) {
	return PyLong_FromLong(self->max_texture_units);
}

char MGLContext_max_texture_units_doc[] = R"(
	max_texture_units

	The max texture units.
)";

PyGetSetDef MGLContext_tp_getseters[] = {
	{(char *)"line_width", 0, (setter)MGLContext_set_line_width, MGLContext_line_width_doc, 0},
	{(char *)"point_size", 0, (setter)MGLContext_set_point_size, MGLContext_point_size_doc, 0},
	{(char *)"viewport", (getter)MGLContext_get_viewport, (setter)MGLContext_set_viewport, MGLContext_viewport_doc, 0},
	{(char *)"default_texture_unit", (getter)MGLContext_get_default_texture_unit, (setter)MGLContext_set_default_texture_unit, MGLContext_default_texture_unit_doc, 0},
	{(char *)"max_texture_units", (getter)MGLContext_get_max_texture_units, 0, MGLContext_max_texture_units_doc, 0},
	{(char *)"vsync", 0, 0, 0, 0},
	{0},
};

const char * MGLContext_tp_doc = R"(
	Context

	An instance can be created using ~ModernGL.create_context.
)";

PyTypeObject MGLContext_Type = {
	PyVarObject_HEAD_INIT(0, 0)
	"ModernGL.Context",                                     // tp_name
	sizeof(MGLContext),                                     // tp_basicsize
	0,                                                      // tp_itemsize
	(destructor)MGLContext_tp_dealloc,                      // tp_dealloc
	0,                                                      // tp_print
	0,                                                      // tp_getattr
	0,                                                      // tp_setattr
	0,                                                      // tp_reserved
	(reprfunc)MGLContext_tp_str,                            // tp_repr
	0,                                                      // tp_as_number
	0,                                                      // tp_as_sequence
	0,                                                      // tp_as_mapping
	0,                                                      // tp_hash
	0,                                                      // tp_call
	(reprfunc)MGLContext_tp_str,                            // tp_str
	0,                                                      // tp_getattro
	0,                                                      // tp_setattro
	0,                                                      // tp_as_buffer
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,               // tp_flags
	MGLContext_tp_doc,                                      // tp_doc
	0,                                                      // tp_traverse
	0,                                                      // tp_clear
	0,                                                      // tp_richcompare
	0,                                                      // tp_weaklistoffset
	0,                                                      // tp_iter
	0,                                                      // tp_iternext
	MGLContext_tp_methods,                                  // tp_methods
	0,                                                      // tp_members
	MGLContext_tp_getseters,                                // tp_getset
	&MGLObject_Type,                                        // tp_base
	0,                                                      // tp_dict
	0,                                                      // tp_descr_get
	0,                                                      // tp_descr_set
	0,                                                      // tp_dictoffset
	(initproc)MGLContext_tp_init,                           // tp_init
	0,                                                      // tp_alloc
	MGLContext_tp_new,                                      // tp_new
};

MGLContext * MGLContext_New() {
	MGLContext * self = (MGLContext *)MGLContext_tp_new(&MGLContext_Type, 0, 0);
	return self;
}

void MGLContext_Invalidate(MGLContext * context) {
	if (Py_TYPE(context) == &MGLInvalidObject_Type) {

		#ifdef MGL_VERBOSE
		printf("MGLContext_Invalidate %p already released\n", context);
		#endif

		return;
	}

	#ifdef MGL_VERBOSE
	printf("MGLContext_Invalidate %p\n", context);
	#endif

	// TODO: destroy standalone context

	context->ob_base.ob_type = &MGLInvalidObject_Type;
	context->initial_type = &MGLContext_Type;

	Py_DECREF(context);
}

void MGLContext_Initialize(MGLContext * self) {
	self->gl.load();

	self->gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// self->gl.Enable(GL_PRIMITIVE_RESTART_FIXED_INDEX);

	self->gl.Enable(GL_PRIMITIVE_RESTART);
	self->gl.PrimitiveRestartIndex(-1);

	self->gl.GetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint *)&self->max_texture_units);
	self->default_texture_unit = self->max_texture_units - 1;
}
