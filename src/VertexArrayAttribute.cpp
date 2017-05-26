#include "VertexArrayAttribute.hpp"

#include "Error.hpp"
#include "Buffer.hpp"

PyObject * MGLVertexArrayAttribute_tp_new(PyTypeObject * type, PyObject * args, PyObject * kwargs) {
	MGLVertexArrayAttribute * self = (MGLVertexArrayAttribute *)type->tp_alloc(type, 0);

	#ifdef MGL_VERBOSE
	printf("MGLVertexArrayAttribute_tp_new %p\n", self);
	#endif

	if (self) {
	}

	return (PyObject *)self;
}

void MGLVertexArrayAttribute_tp_dealloc(MGLVertexArrayAttribute * self) {

	#ifdef MGL_VERBOSE
	printf("MGLVertexArrayAttribute_tp_dealloc %p\n", self);
	#endif

	Py_TYPE(self)->tp_free((PyObject *)self);
}

int MGLVertexArrayAttribute_tp_init(MGLVertexArrayAttribute * self, PyObject * args, PyObject * kwargs) {
	MGLError_Set("cannot create mgl.VertexArrayAttribute manually");
	return -1;
}

PyObject * MGLVertexArrayAttribute_bind(MGLVertexArrayAttribute * self, PyObject * args) {
	MGLBuffer * buffer;
	int offset;
	int stride;
	int divisor;

	int args_ok = PyArg_ParseTuple(
		args,
		"O!III",
		&MGLBuffer_Type,
		&buffer,
		&offset,
		&stride,
		&divisor
	);

	if (!args_ok) {
		return 0;
	}

	self->gl->BindVertexArray(self->vertex_array_obj);
	self->gl->BindBuffer(GL_ARRAY_BUFFER, buffer->buffer_obj);

	if (self->normalizable) {
		((gl_attribute_normal_ptr_proc)self->gl_attrib_ptr_proc)(self->location, self->row_length, self->scalar_type, false, stride, (void *)(long long)offset);
	} else {
		((gl_attribute_ptr_proc)self->gl_attrib_ptr_proc)(self->location, self->row_length, self->scalar_type, stride, (void *)(long long)offset);
	}

	self->gl->VertexAttribDivisor(self->location, divisor);
	self->gl->EnableVertexAttribArray(self->location);

	Py_RETURN_NONE;
}

PyObject * MGLVertexArrayAttribute_enable(MGLVertexArrayAttribute * self) {
	self->gl->BindVertexArray(self->vertex_array_obj);
	self->gl->EnableVertexAttribArray(self->location);
	Py_RETURN_NONE;
}

PyObject * MGLVertexArrayAttribute_disable(MGLVertexArrayAttribute * self) {
	self->gl->BindVertexArray(self->vertex_array_obj);
	self->gl->DisableVertexAttribArray(self->location);
	Py_RETURN_NONE;
}

PyMethodDef MGLVertexArrayAttribute_tp_methods[] = {
	{"bind", (PyCFunction)MGLVertexArrayAttribute_bind, METH_VARARGS, 0},
	{"enable", (PyCFunction)MGLVertexArrayAttribute_enable, METH_NOARGS, 0},
	{"disable", (PyCFunction)MGLVertexArrayAttribute_disable, METH_NOARGS, 0},
	{0},
};

PyObject * MGLVertexArrayAttribute_get_location(MGLVertexArrayAttribute * self, void * closure) {
	return PyLong_FromLong(self->location);
}

PyObject * MGLVertexArrayAttribute_get_divisor(MGLVertexArrayAttribute * self, void * closure) {
	int divisor = 0;
	self->gl->BindVertexArray(self->vertex_array_obj);
	self->gl->GetVertexAttribiv(self->location, GL_VERTEX_ATTRIB_ARRAY_DIVISOR, &divisor);
	return PyLong_FromLong(divisor);
}

PyObject * MGLVertexArrayAttribute_get_stride(MGLVertexArrayAttribute * self, void * closure) {
	int stride = 0;
	self->gl->BindVertexArray(self->vertex_array_obj);
	self->gl->GetVertexAttribiv(self->location, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
	return PyLong_FromLong(stride);
}

PyObject * MGLVertexArrayAttribute_get_enabled(MGLVertexArrayAttribute * self, void * closure) {
	int enabled = 0;
	self->gl->BindVertexArray(self->vertex_array_obj);
	self->gl->GetVertexAttribiv(self->location, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
	return PyBool_FromLong(enabled);
}

int MGLVertexArrayAttribute_set_enabled(MGLVertexArrayAttribute * self, PyObject * value, void * closure) {
	if (value == Py_True) {
		self->gl->BindVertexArray(self->vertex_array_obj);
		self->gl->EnableVertexAttribArray(self->location);
	} else if (value == Py_False) {
		self->gl->BindVertexArray(self->vertex_array_obj);
		self->gl->DisableVertexAttribArray(self->location);
	} else {
		MGLError_Set("invalid value for enabled");
		return -1;
	}
	return 0;
}

PyGetSetDef MGLVertexArrayAttribute_tp_getseters[] = {
	{(char *)"location", (getter)MGLVertexArrayAttribute_get_location, 0, 0, 0},
	{(char *)"divisor", (getter)MGLVertexArrayAttribute_get_divisor, 0, 0, 0},
	{(char *)"stride", (getter)MGLVertexArrayAttribute_get_stride, 0, 0, 0},
	{(char *)"enabled", (getter)MGLVertexArrayAttribute_get_enabled, 0, 0, 0},
	{0},
};

PyTypeObject MGLVertexArrayAttribute_Type = {
	PyVarObject_HEAD_INIT(0, 0)
	"mgl.VertexArrayAttribute",                             // tp_name
	sizeof(MGLVertexArrayAttribute),                        // tp_basicsize
	0,                                                      // tp_itemsize
	(destructor)MGLVertexArrayAttribute_tp_dealloc,         // tp_dealloc
	0,                                                      // tp_print
	0,                                                      // tp_getattr
	0,                                                      // tp_setattr
	0,                                                      // tp_reserved
	0,                                                      // tp_repr
	0,                                                      // tp_as_number
	0,                                                      // tp_as_sequence
	0,                                                      // tp_as_mapping
	0,                                                      // tp_hash
	0,                                                      // tp_call
	0,                                                      // tp_str
	0,                                                      // tp_getattro
	0,                                                      // tp_setattro
	0,                                                      // tp_as_buffer
	Py_TPFLAGS_DEFAULT,                                     // tp_flags
	0,                                                      // tp_doc
	0,                                                      // tp_traverse
	0,                                                      // tp_clear
	0,                                                      // tp_richcompare
	0,                                                      // tp_weaklistoffset
	0,                                                      // tp_iter
	0,                                                      // tp_iternext
	MGLVertexArrayAttribute_tp_methods,                     // tp_methods
	0,                                                      // tp_members
	MGLVertexArrayAttribute_tp_getseters,                   // tp_getset
	0,                                                      // tp_base
	0,                                                      // tp_dict
	0,                                                      // tp_descr_get
	0,                                                      // tp_descr_set
	0,                                                      // tp_dictoffset
	(initproc)MGLVertexArrayAttribute_tp_init,              // tp_init
	0,                                                      // tp_alloc
	MGLVertexArrayAttribute_tp_new,                         // tp_new
};

MGLVertexArrayAttribute * MGLVertexArrayAttribute_New() {
	MGLVertexArrayAttribute * self = (MGLVertexArrayAttribute *)MGLVertexArrayAttribute_tp_new(&MGLVertexArrayAttribute_Type, 0, 0);
	return self;
}

void MGLVertexArrayAttribute_Complete(MGLVertexArrayAttribute * attribute, const GLMethods & gl) {
	switch (attribute->type) {
		case GL_INT:
			attribute->normalizable = false;
			attribute->row_length = 1;
			attribute->scalar_type = GL_INT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribIPointer;
			break;

		case GL_INT_VEC2:
			attribute->normalizable = false;
			attribute->row_length = 2;
			attribute->scalar_type = GL_INT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribIPointer;
			break;

		case GL_INT_VEC3:
			attribute->normalizable = false;
			attribute->row_length = 3;
			attribute->scalar_type = GL_INT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribIPointer;
			break;

		case GL_INT_VEC4:
			attribute->normalizable = false;
			attribute->row_length = 4;
			attribute->scalar_type = GL_INT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribIPointer;
			break;

		case GL_UNSIGNED_INT:
			attribute->normalizable = false;
			attribute->row_length = 1;
			attribute->scalar_type = GL_UNSIGNED_INT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribIPointer;
			break;

		case GL_UNSIGNED_INT_VEC2:
			attribute->normalizable = false;
			attribute->row_length = 2;
			attribute->scalar_type = GL_UNSIGNED_INT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribIPointer;
			break;

		case GL_UNSIGNED_INT_VEC3:
			attribute->normalizable = false;
			attribute->row_length = 3;
			attribute->scalar_type = GL_UNSIGNED_INT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribIPointer;
			break;

		case GL_UNSIGNED_INT_VEC4:
			attribute->normalizable = false;
			attribute->row_length = 4;
			attribute->scalar_type = GL_UNSIGNED_INT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribIPointer;
			break;

		case GL_FLOAT:
			attribute->normalizable = true;
			attribute->row_length = 1;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;

		case GL_FLOAT_VEC2:
			attribute->normalizable = true;
			attribute->row_length = 2;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;

		case GL_FLOAT_VEC3:
			attribute->normalizable = true;
			attribute->row_length = 3;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;

		case GL_FLOAT_VEC4:
			attribute->normalizable = true;
			attribute->row_length = 4;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;

		case GL_DOUBLE:
			attribute->normalizable = false;
			attribute->row_length = 1;
			attribute->scalar_type = GL_DOUBLE;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribLPointer;
			break;

		case GL_DOUBLE_VEC2:
			attribute->normalizable = false;
			attribute->row_length = 2;
			attribute->scalar_type = GL_DOUBLE;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribLPointer;
			break;

		case GL_DOUBLE_VEC3:
			attribute->normalizable = false;
			attribute->row_length = 3;
			attribute->scalar_type = GL_DOUBLE;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribLPointer;
			break;

		case GL_DOUBLE_VEC4:
			attribute->normalizable = false;
			attribute->row_length = 4;
			attribute->scalar_type = GL_DOUBLE;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribLPointer;
			break;

		case GL_FLOAT_MAT2:
			attribute->normalizable = true;
			attribute->row_length = 2;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;

		case GL_FLOAT_MAT2x3:
			attribute->normalizable = true;
			attribute->row_length = 3;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;

		case GL_FLOAT_MAT2x4:
			attribute->normalizable = true;
			attribute->row_length = 4;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;

		case GL_FLOAT_MAT3x2:
			attribute->normalizable = true;
			attribute->row_length = 2;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;

		case GL_FLOAT_MAT3:
			attribute->normalizable = true;
			attribute->row_length = 3;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;

		case GL_FLOAT_MAT3x4:
			attribute->normalizable = true;
			attribute->row_length = 4;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;

		case GL_FLOAT_MAT4x2:
			attribute->normalizable = true;
			attribute->row_length = 2;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;

		case GL_FLOAT_MAT4x3:
			attribute->normalizable = true;
			attribute->row_length = 3;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;

		case GL_FLOAT_MAT4:
			attribute->normalizable = true;
			attribute->row_length = 4;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;

		case GL_DOUBLE_MAT2:
			attribute->normalizable = false;
			attribute->row_length = 2;
			attribute->scalar_type = GL_DOUBLE;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribLPointer;
			break;

		case GL_DOUBLE_MAT2x3:
			attribute->normalizable = false;
			attribute->row_length = 3;
			attribute->scalar_type = GL_DOUBLE;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribLPointer;
			break;

		case GL_DOUBLE_MAT2x4:
			attribute->normalizable = false;
			attribute->row_length = 4;
			attribute->scalar_type = GL_DOUBLE;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribLPointer;
			break;

		case GL_DOUBLE_MAT3x2:
			attribute->normalizable = false;
			attribute->row_length = 2;
			attribute->scalar_type = GL_DOUBLE;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribLPointer;
			break;

		case GL_DOUBLE_MAT3:
			attribute->normalizable = false;
			attribute->row_length = 3;
			attribute->scalar_type = GL_DOUBLE;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribLPointer;
			break;

		case GL_DOUBLE_MAT3x4:
			attribute->normalizable = false;
			attribute->row_length = 4;
			attribute->scalar_type = GL_DOUBLE;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribLPointer;
			break;

		case GL_DOUBLE_MAT4x2:
			attribute->normalizable = false;
			attribute->row_length = 2;
			attribute->scalar_type = GL_DOUBLE;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribLPointer;
			break;

		case GL_DOUBLE_MAT4x3:
			attribute->normalizable = false;
			attribute->row_length = 3;
			attribute->scalar_type = GL_DOUBLE;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribLPointer;
			break;

		case GL_DOUBLE_MAT4:
			attribute->normalizable = false;
			attribute->row_length = 4;
			attribute->scalar_type = GL_DOUBLE;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribLPointer;
			break;

		default:
			attribute->normalizable = false;
			attribute->row_length = 1;
			attribute->scalar_type = GL_FLOAT;

			attribute->gl_attrib_ptr_proc = (void *)gl.VertexAttribPointer;
			break;
	}
}
