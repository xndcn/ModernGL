#include "Uniform.hpp"

#include "Error.hpp"
#include "InvalidObject.hpp"

PyObject * MGLUniform_tp_new(PyTypeObject * type, PyObject * args, PyObject * kwargs) {
	MGLUniform * self = (MGLUniform *)type->tp_alloc(type, 0);

	#ifdef MGL_VERBOSE
	printf("MGLUniform_tp_new %p\n", self);
	#endif

	if (self) {
	}

	return (PyObject *)self;
}

void MGLUniform_tp_dealloc(MGLUniform * self) {

	#ifdef MGL_VERBOSE
	printf("MGLUniform_tp_dealloc %p\n", self);
	#endif

	MGLUniform_Type.tp_free((PyObject *)self);
}

int MGLUniform_tp_init(MGLUniform * self, PyObject * args, PyObject * kwargs) {
	MGLError_Set("cannot create mgl.Uniform manually");
	return -1;
}

PyObject * MGLUniform_read(MGLUniform * self) {
	PyObject * result = PyBytes_FromStringAndSize(0, self->element_size);
	char * data = PyBytes_AS_STRING(result);
	((gl_uniform_reader_proc)self->gl_value_reader_proc)(self->program_obj, self->location, data);
	MGL_GLMETHOD_DBG_UNIFORM_READER(self, self->location, data);
	return result;
}

PyObject * MGLUniform_write(MGLUniform * self, PyObject * args) {
	const char * buffer;
	Py_ssize_t size;

	int args_ok = PyArg_ParseTuple(
		args,
		"y#",   // TODO: replace y# with O if possible
		&buffer,
		&size
	);

	if (!args_ok) {
		return 0;
	}

	if (size != self->array_length * self->element_size) {
		MGLError_Set("data size mismatch %d != %d", size, self->array_length * self->element_size);
		return 0;
	}

	const GLMethods & gl = self->context->gl;
	gl.UseProgram(self->program_obj);
	if (self->matrix) {
		((gl_uniform_matrix_writer_proc)self->gl_value_writer_proc)(self->location, self->array_length, false, buffer);
		MGL_GLMETHOD_DBG_UNIFORM_WRITER(self, self->location, self->array_length, buffer);
	} else {
		((gl_uniform_vector_writer_proc)self->gl_value_writer_proc)(self->location, self->array_length, buffer);
		MGL_GLMETHOD_DBG_UNIFORM_WRITER(self, self->location, self->array_length, buffer);
	}

	Py_RETURN_NONE;
}

PyMethodDef MGLUniform_tp_methods[] = {
	{"read", (PyCFunction)MGLUniform_read, METH_NOARGS, 0},
	{"write", (PyCFunction)MGLUniform_write, METH_VARARGS, 0},
	{0},
};

PyObject * MGLUniform_get_name(MGLUniform * self, void * closure) {
	Py_INCREF(self->name);
	return self->name;
}

PyObject * MGLUniform_get_location(MGLUniform * self, void * closure) {
	return PyLong_FromLong(self->location);
}

PyObject * MGLUniform_get_dimension(MGLUniform * self, void * closure) {
	return PyLong_FromLong(self->dimension);
}

PyObject * MGLUniform_get_value(MGLUniform * self, void * closure) {
	return ((MGLUniform_Getter)self->value_getter)(self);
}

int MGLUniform_set_value(MGLUniform * self, PyObject * value, void * closure) {
	return ((MGLUniform_Setter)self->value_setter)(self, value);
}

PyObject * MGLUniform_get_array_length(MGLUniform * self, void * closure) {
	return PyLong_FromLong(self->array_length);
}

PyGetSetDef MGLUniform_tp_getseters[] = {
	{(char *)"name", (getter)MGLUniform_get_name, 0, 0, 0},
	{(char *)"location", (getter)MGLUniform_get_location, 0, 0, 0},
	{(char *)"dimension", (getter)MGLUniform_get_dimension, 0, 0, 0},
	{(char *)"value", (getter)MGLUniform_get_value, (setter)MGLUniform_set_value, 0, 0},
	{(char *)"array_length", (getter)MGLUniform_get_array_length, 0, 0, 0},
	{0},
};

PyTypeObject MGLUniform_Type = {
	PyVarObject_HEAD_INIT(0, 0)
	"mgl.Uniform",                                          // tp_name
	sizeof(MGLUniform),                                     // tp_basicsize
	0,                                                      // tp_itemsize
	(destructor)MGLUniform_tp_dealloc,                      // tp_dealloc
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
	MGLUniform_tp_methods,                                  // tp_methods
	0,                                                      // tp_members
	MGLUniform_tp_getseters,                                // tp_getset
	0,                                                      // tp_base
	0,                                                      // tp_dict
	0,                                                      // tp_descr_get
	0,                                                      // tp_descr_set
	0,                                                      // tp_dictoffset
	(initproc)MGLUniform_tp_init,                           // tp_init
	0,                                                      // tp_alloc
	MGLUniform_tp_new,                                      // tp_new
};

MGLUniform * MGLUniform_New() {
	MGLUniform * self = (MGLUniform *)MGLUniform_tp_new(&MGLUniform_Type, 0, 0);
	return self;
}

void MGLUniform_Invalidate(MGLUniform * uniform) {

	#ifdef MGL_VERBOSE
	printf("MGLUniform_Invalidate %p\n", uniform);
	#endif

	Py_DECREF(uniform->name);

	Py_TYPE(uniform) = &MGLInvalidObject_Type;

	Py_DECREF(uniform);
}

void MGLUniform_Complete(MGLUniform * self, const GLMethods & gl) {
	switch (self->type) {
		case GL_BOOL:
			self->matrix = false;
			self->dimension = 1;
			self->element_size = 4;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform1iv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_bool_array_value_getter;
				self->value_setter = (MGLProc)MGLUniform_bool_array_value_setter;
			} else {
				self->value_getter = (MGLProc)MGLUniform_bool_value_getter;
				self->value_setter = (MGLProc)MGLUniform_bool_value_setter;
			}
			break;

		case GL_BOOL_VEC2:
			self->matrix = false;
			self->dimension = 2;
			self->element_size = 8;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform2iv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_bvec_array_value_getter<2>;
				self->value_setter = (MGLProc)MGLUniform_bvec_array_value_setter<2>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_bvec_value_getter<2>;
				self->value_setter = (MGLProc)MGLUniform_bvec_value_setter<2>;
			}
			break;

		case GL_BOOL_VEC3:
			self->matrix = false;
			self->dimension = 3;
			self->element_size = 12;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform3iv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_bvec_array_value_getter<3>;
				self->value_setter = (MGLProc)MGLUniform_bvec_array_value_setter<3>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_bvec_value_getter<3>;
				self->value_setter = (MGLProc)MGLUniform_bvec_value_setter<3>;
			}
			break;

		case GL_BOOL_VEC4:
			self->matrix = false;
			self->dimension = 4;
			self->element_size = 16;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform4iv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_bvec_array_value_getter<4>;
				self->value_setter = (MGLProc)MGLUniform_bvec_array_value_setter<4>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_bvec_value_getter<4>;
				self->value_setter = (MGLProc)MGLUniform_bvec_value_setter<4>;
			}
			break;

		case GL_INT:
			self->matrix = false;
			self->dimension = 1;
			self->element_size = 4;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform1iv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_int_array_value_getter;
				self->value_setter = (MGLProc)MGLUniform_int_array_value_setter;
			} else {
				self->value_getter = (MGLProc)MGLUniform_int_value_getter;
				self->value_setter = (MGLProc)MGLUniform_int_value_setter;
			}
			break;

		case GL_INT_VEC2:
			self->matrix = false;
			self->dimension = 2;
			self->element_size = 8;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform2iv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_ivec_array_value_getter<2>;
				self->value_setter = (MGLProc)MGLUniform_ivec_array_value_setter<2>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_ivec_value_getter<2>;
				self->value_setter = (MGLProc)MGLUniform_ivec_value_setter<2>;
			}
			break;

		case GL_INT_VEC3:
			self->matrix = false;
			self->dimension = 3;
			self->element_size = 12;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform3iv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_ivec_array_value_getter<3>;
				self->value_setter = (MGLProc)MGLUniform_ivec_array_value_setter<3>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_ivec_value_getter<3>;
				self->value_setter = (MGLProc)MGLUniform_ivec_value_setter<3>;
			}
			break;

		case GL_INT_VEC4:
			self->matrix = false;
			self->dimension = 4;
			self->element_size = 16;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform4iv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_ivec_array_value_getter<4>;
				self->value_setter = (MGLProc)MGLUniform_ivec_array_value_setter<4>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_ivec_value_getter<4>;
				self->value_setter = (MGLProc)MGLUniform_ivec_value_setter<4>;
			}
			break;

		case GL_UNSIGNED_INT:
			self->matrix = false;
			self->dimension = 1;
			self->element_size = 4;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformuiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform1uiv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_uint_array_value_getter;
				self->value_setter = (MGLProc)MGLUniform_uint_array_value_setter;
			} else {
				self->value_getter = (MGLProc)MGLUniform_uint_value_getter;
				self->value_setter = (MGLProc)MGLUniform_uint_value_setter;
			}
			break;

		case GL_UNSIGNED_INT_VEC2:
			self->matrix = false;
			self->dimension = 2;
			self->element_size = 8;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformuiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform2uiv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_uvec_array_value_getter<2>;
				self->value_setter = (MGLProc)MGLUniform_uvec_array_value_setter<2>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_uvec_value_getter<2>;
				self->value_setter = (MGLProc)MGLUniform_uvec_value_setter<2>;
			}
			break;

		case GL_UNSIGNED_INT_VEC3:
			self->matrix = false;
			self->dimension = 3;
			self->element_size = 12;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformuiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform3uiv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_uvec_array_value_getter<3>;
				self->value_setter = (MGLProc)MGLUniform_uvec_array_value_setter<3>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_uvec_value_getter<3>;
				self->value_setter = (MGLProc)MGLUniform_uvec_value_setter<3>;
			}
			break;

		case GL_UNSIGNED_INT_VEC4:
			self->matrix = false;
			self->dimension = 4;
			self->element_size = 16;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformuiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform4uiv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_uvec_array_value_getter<4>;
				self->value_setter = (MGLProc)MGLUniform_uvec_array_value_setter<4>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_uvec_value_getter<4>;
				self->value_setter = (MGLProc)MGLUniform_uvec_value_setter<4>;
			}
			break;

		case GL_FLOAT:
			self->matrix = false;
			self->dimension = 1;
			self->element_size = 4;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform1fv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_float_array_value_getter;
				self->value_setter = (MGLProc)MGLUniform_float_array_value_setter;
			} else {
				self->value_getter = (MGLProc)MGLUniform_float_value_getter;
				self->value_setter = (MGLProc)MGLUniform_float_value_setter;
			}
			break;

		case GL_FLOAT_VEC2:
			self->matrix = false;
			self->dimension = 2;
			self->element_size = 8;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform2fv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_vec_array_value_getter<2>;
				self->value_setter = (MGLProc)MGLUniform_vec_array_value_setter<2>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_vec_value_getter<2>;
				self->value_setter = (MGLProc)MGLUniform_vec_value_setter<2>;
			}
			break;

		case GL_FLOAT_VEC3:
			self->matrix = false;
			self->dimension = 3;
			self->element_size = 12;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform3fv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_vec_array_value_getter<3>;
				self->value_setter = (MGLProc)MGLUniform_vec_array_value_setter<3>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_vec_value_getter<3>;
				self->value_setter = (MGLProc)MGLUniform_vec_value_setter<3>;
			}
			break;

		case GL_FLOAT_VEC4:
			self->matrix = false;
			self->dimension = 4;
			self->element_size = 16;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform4fv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_vec_array_value_getter<4>;
				self->value_setter = (MGLProc)MGLUniform_vec_array_value_setter<4>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_vec_value_getter<4>;
				self->value_setter = (MGLProc)MGLUniform_vec_value_setter<4>;
			}
			break;

		case GL_DOUBLE:
			self->matrix = false;
			self->dimension = 1;
			self->element_size = 8;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformdv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform1dv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_double_array_value_getter;
				self->value_setter = (MGLProc)MGLUniform_double_array_value_setter;
			} else {
				self->value_getter = (MGLProc)MGLUniform_double_value_getter;
				self->value_setter = (MGLProc)MGLUniform_double_value_setter;
			}
			break;

		case GL_DOUBLE_VEC2:
			self->matrix = false;
			self->dimension = 2;
			self->element_size = 16;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformdv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform2dv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_dvec_array_value_getter<2>;
				self->value_setter = (MGLProc)MGLUniform_dvec_array_value_setter<2>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_dvec_value_getter<2>;
				self->value_setter = (MGLProc)MGLUniform_dvec_value_setter<2>;
			}
			break;

		case GL_DOUBLE_VEC3:
			self->matrix = false;
			self->dimension = 3;
			self->element_size = 24;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformdv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform3dv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_dvec_array_value_getter<3>;
				self->value_setter = (MGLProc)MGLUniform_dvec_array_value_setter<3>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_dvec_value_getter<3>;
				self->value_setter = (MGLProc)MGLUniform_dvec_value_setter<3>;
			}
			break;

		case GL_DOUBLE_VEC4:
			self->matrix = false;
			self->dimension = 4;
			self->element_size = 32;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformdv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform4dv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_dvec_array_value_getter<4>;
				self->value_setter = (MGLProc)MGLUniform_dvec_array_value_setter<4>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_dvec_value_getter<4>;
				self->value_setter = (MGLProc)MGLUniform_dvec_value_setter<4>;
			}
			break;

		case GL_SAMPLER_2D:
			self->matrix = false;
			self->dimension = 1;
			self->element_size = 4;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform1iv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_sampler_array_value_getter;
				self->value_setter = (MGLProc)MGLUniform_sampler_array_value_setter;
			} else {
				self->value_getter = (MGLProc)MGLUniform_sampler_value_getter;
				self->value_setter = (MGLProc)MGLUniform_sampler_value_setter;
			}
			break;

		case GL_SAMPLER_3D:
			self->matrix = false;
			self->dimension = 1;
			self->element_size = 4;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform1iv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_sampler_array_value_getter;
				self->value_setter = (MGLProc)MGLUniform_sampler_array_value_setter;
			} else {
				self->value_getter = (MGLProc)MGLUniform_sampler_value_getter;
				self->value_setter = (MGLProc)MGLUniform_sampler_value_setter;
			}
			break;

		case GL_SAMPLER_2D_SHADOW:
			self->matrix = false;
			self->dimension = 1;
			self->element_size = 4;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform1iv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_sampler_array_value_getter;
				self->value_setter = (MGLProc)MGLUniform_sampler_array_value_setter;
			} else {
				self->value_getter = (MGLProc)MGLUniform_sampler_value_getter;
				self->value_setter = (MGLProc)MGLUniform_sampler_value_setter;
			}
			break;

		case GL_SAMPLER_2D_MULTISAMPLE:
			self->matrix = false;
			self->dimension = 1;
			self->element_size = 4;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform1iv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_sampler_array_value_getter;
				self->value_setter = (MGLProc)MGLUniform_sampler_array_value_setter;
			} else {
				self->value_getter = (MGLProc)MGLUniform_sampler_value_getter;
				self->value_setter = (MGLProc)MGLUniform_sampler_value_setter;
			}
			break;

		case GL_SAMPLER_CUBE:
			self->matrix = false;
			self->dimension = 1;
			self->element_size = 4;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformiv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform1iv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_sampler_array_value_getter;
				self->value_setter = (MGLProc)MGLUniform_sampler_array_value_setter;
			} else {
				self->value_getter = (MGLProc)MGLUniform_sampler_value_getter;
				self->value_setter = (MGLProc)MGLUniform_sampler_value_setter;
			}
			break;

		case GL_FLOAT_MAT2:
			self->matrix = true;
			self->dimension = 4;
			self->element_size = 16;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix2fv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<float, 2, 2>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<float, 2, 2>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<float, 2, 2>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<float, 2, 2>;
			}
			break;

		case GL_FLOAT_MAT2x3:
			self->matrix = true;
			self->dimension = 6;
			self->element_size = 24;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix2x3fv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<float, 2, 3>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<float, 2, 3>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<float, 2, 3>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<float, 2, 3>;
			}
			break;

		case GL_FLOAT_MAT2x4:
			self->matrix = true;
			self->dimension = 8;
			self->element_size = 32;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix2x4fv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<float, 2, 4>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<float, 2, 4>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<float, 2, 4>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<float, 2, 4>;
			}
			break;

		case GL_FLOAT_MAT3x2:
			self->matrix = true;
			self->dimension = 6;
			self->element_size = 24;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix3x2fv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<float, 3, 2>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<float, 3, 2>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<float, 3, 2>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<float, 3, 2>;
			}
			break;

		case GL_FLOAT_MAT3:
			self->matrix = true;
			self->dimension = 9;
			self->element_size = 36;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix3fv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<float, 3, 3>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<float, 3, 3>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<float, 3, 3>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<float, 3, 3>;
			}
			break;

		case GL_FLOAT_MAT3x4:
			self->matrix = true;
			self->dimension = 12;
			self->element_size = 48;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix3x4fv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<float, 3, 4>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<float, 3, 4>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<float, 3, 4>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<float, 3, 4>;
			}
			break;

		case GL_FLOAT_MAT4x2:
			self->matrix = true;
			self->dimension = 8;
			self->element_size = 32;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix4x2fv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<float, 4, 2>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<float, 4, 2>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<float, 4, 2>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<float, 4, 2>;
			}
			break;

		case GL_FLOAT_MAT4x3:
			self->matrix = true;
			self->dimension = 12;
			self->element_size = 48;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix4x3fv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<float, 4, 3>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<float, 4, 3>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<float, 4, 3>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<float, 4, 3>;
			}
			break;

		case GL_FLOAT_MAT4:
			self->matrix = true;
			self->dimension = 16;
			self->element_size = 64;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix4fv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<float, 4, 4>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<float, 4, 4>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<float, 4, 4>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<float, 4, 4>;
			}
			break;

		case GL_DOUBLE_MAT2:
			self->matrix = true;
			self->dimension = 4;
			self->element_size = 32;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformdv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix2dv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<double, 2, 2>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<double, 2, 2>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<double, 2, 2>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<double, 2, 2>;
			}
			break;

		case GL_DOUBLE_MAT2x3:
			self->matrix = true;
			self->dimension = 6;
			self->element_size = 48;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformdv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix2x3dv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<double, 2, 3>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<double, 2, 3>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<double, 2, 3>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<double, 2, 3>;
			}
			break;

		case GL_DOUBLE_MAT2x4:
			self->matrix = true;
			self->dimension = 8;
			self->element_size = 64;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformdv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix2x4dv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<double, 2, 4>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<double, 2, 4>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<double, 2, 4>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<double, 2, 4>;
			}
			break;

		case GL_DOUBLE_MAT3x2:
			self->matrix = true;
			self->dimension = 6;
			self->element_size = 48;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformdv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix3x2dv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<double, 3, 2>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<double, 3, 2>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<double, 3, 2>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<double, 3, 2>;
			}
			break;

		case GL_DOUBLE_MAT3:
			self->matrix = true;
			self->dimension = 9;
			self->element_size = 72;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformdv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix3dv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<double, 3, 3>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<double, 3, 3>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<double, 3, 3>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<double, 3, 3>;
			}
			break;

		case GL_DOUBLE_MAT3x4:
			self->matrix = true;
			self->dimension = 12;
			self->element_size = 96;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformdv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix3x4dv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<double, 3, 4>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<double, 3, 4>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<double, 3, 4>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<double, 3, 4>;
			}
			break;

		case GL_DOUBLE_MAT4x2:
			self->matrix = true;
			self->dimension = 8;
			self->element_size = 64;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformdv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix4x2dv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<double, 4, 2>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<double, 4, 2>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<double, 4, 2>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<double, 4, 2>;
			}
			break;

		case GL_DOUBLE_MAT4x3:
			self->matrix = true;
			self->dimension = 12;
			self->element_size = 96;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformdv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix4x3dv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<double, 4, 3>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<double, 4, 3>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<double, 4, 3>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<double, 4, 3>;
			}
			break;

		case GL_DOUBLE_MAT4:
			self->matrix = true;
			self->dimension = 16;
			self->element_size = 128;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformdv;
			self->gl_value_writer_proc = (MGLProc)gl.UniformMatrix4dv;
			if (self->array_length > 1) {
				self->value_getter = (MGLProc)MGLUniform_matrix_array_value_getter<double, 4, 4>;
				self->value_setter = (MGLProc)MGLUniform_matrix_array_value_setter<double, 4, 4>;
			} else {
				self->value_getter = (MGLProc)MGLUniform_matrix_value_getter<double, 4, 4>;
				self->value_setter = (MGLProc)MGLUniform_matrix_value_setter<double, 4, 4>;
			}
			break;

		default:
			self->matrix = false;
			self->dimension = 1;
			self->element_size = 4;
			self->gl_value_reader_proc = (MGLProc)gl.GetUniformfv;
			self->gl_value_writer_proc = (MGLProc)gl.Uniform1fv;
			self->value_getter = (MGLProc)MGLUniform_invalid_getter;
			self->value_setter = (MGLProc)MGLUniform_invalid_setter;
			break;
	}
}
