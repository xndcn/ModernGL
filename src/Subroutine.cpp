#include "Subroutine.hpp"

#include "InvalidObject.hpp"

PyObject * MGLSubroutine_tp_new(PyTypeObject * type, PyObject * args, PyObject * kwargs) {
	MGLSubroutine * self = (MGLSubroutine *)type->tp_alloc(type, 0);

	#ifdef MGL_VERBOSE
	printf("MGLSubroutine_tp_new %p\n", self);
	#endif

	if (self) {
		self->name = 0;
	}

	return (PyObject *)self;
}

void MGLSubroutine_tp_dealloc(MGLSubroutine * self) {

	#ifdef MGL_VERBOSE
	printf("MGLSubroutine_tp_dealloc %p\n", self);
	#endif

	Py_XDECREF(self->name);

	Py_TYPE(self)->tp_free((PyObject*)self);
}

int MGLSubroutine_tp_init(MGLSubroutine * self, PyObject * args, PyObject * kwargs) {
	return -1;
}

PyObject * MGLSubroutine_tp_str(MGLSubroutine * self) {
	return PyUnicode_FromFormat("<ModernGL.Subroutine>");
}

PyMethodDef MGLSubroutine_tp_methods[] = {
	{0},
};

PyObject * MGLSubroutine_get_name(MGLSubroutine * self, void * closure) {
	Py_INCREF(self->name);
	return self->name;
}

char MGLSubroutine_name_doc[] = R"(
	name
)";

PyObject * MGLSubroutine_get_location(MGLSubroutine * self, void * closure) {
	return PyLong_FromLong(self->location);
}

char MGLSubroutine_location_doc[] = R"(
	location
)";

PyGetSetDef MGLSubroutine_tp_getseters[] = {
	{(char *)"name", (getter)MGLSubroutine_get_name, 0, MGLSubroutine_name_doc, 0},
	{(char *)"location", (getter)MGLSubroutine_get_location, 0, MGLSubroutine_location_doc, 0},
	{0},
};

const char * MGLSubroutine_tp_doc = R"(
	Subroutine
)";

PyTypeObject MGLSubroutine_Type = {
	PyVarObject_HEAD_INIT(0, 0)
	"ModernGL.Subroutine",                                  // tp_name
	sizeof(MGLSubroutine),                                  // tp_basicsize
	0,                                                      // tp_itemsize
	(destructor)MGLSubroutine_tp_dealloc,                   // tp_dealloc
	0,                                                      // tp_print
	0,                                                      // tp_getattr
	0,                                                      // tp_setattr
	0,                                                      // tp_reserved
	(reprfunc)MGLSubroutine_tp_str,                         // tp_repr
	0,                                                      // tp_as_number
	0,                                                      // tp_as_sequence
	0,                                                      // tp_as_mapping
	0,                                                      // tp_hash
	0,                                                      // tp_call
	(reprfunc)MGLSubroutine_tp_str,                         // tp_str
	0,                                                      // tp_getattro
	0,                                                      // tp_setattro
	0,                                                      // tp_as_buffer
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,               // tp_flags
	MGLSubroutine_tp_doc,                                   // tp_doc
	0,                                                      // tp_traverse
	0,                                                      // tp_clear
	0,                                                      // tp_richcompare
	0,                                                      // tp_weaklistoffset
	0,                                                      // tp_iter
	0,                                                      // tp_iternext
	MGLSubroutine_tp_methods,                               // tp_methods
	0,                                                      // tp_members
	MGLSubroutine_tp_getseters,                             // tp_getset
	&MGLProgramMember_Type,                                 // tp_base
	0,                                                      // tp_dict
	0,                                                      // tp_descr_get
	0,                                                      // tp_descr_set
	0,                                                      // tp_dictoffset
	(initproc)MGLSubroutine_tp_init,                        // tp_init
	0,                                                      // tp_alloc
	MGLSubroutine_tp_new,                                   // tp_new
};

MGLSubroutine * MGLSubroutine_New() {
	MGLSubroutine * self = (MGLSubroutine *)MGLSubroutine_tp_new(&MGLSubroutine_Type, 0, 0);
	return self;
}

void MGLSubroutine_Invalidate(MGLSubroutine * subroutine) {

	#ifdef MGL_VERBOSE
	printf("MGLSubroutine_Invalidate %p\n", subroutine);
	#endif

	Py_DECREF(subroutine->name);

	subroutine->ob_base.ob_type = &MGLInvalidObject_Type;
	subroutine->initial_type = &MGLSubroutine_Type;

	Py_DECREF(subroutine);
}
