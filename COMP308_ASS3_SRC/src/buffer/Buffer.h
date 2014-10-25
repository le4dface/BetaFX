
#ifndef BUFFER_H_
#define BUFFER_H_

#include <functional>

namespace std {

class BufferBase {
public:
	virtual ~BufferBase() {}
};

template<class T> class Buffer: public BufferBase {
public:
	GLenum type;
	GLuint location;
	T *data;

	/*
	 * require some function returning how many items T uses
	 */
	function<GLsizeiptr()> sizeFunc;

	Buffer(GLenum t) {
		type = t;
		data = new T();
		sizeFunc = []() -> GLsizeiptr { return 1; };
		glGenBuffers(1, &location);
	}

	Buffer(GLenum t, T *initial, function<GLsizeiptr()> f) {
		type = t;
		data = initial;
		sizeFunc = f;
		glGenBuffers(1, &location);

		// initialise buffer data
		glBindBuffer(type, location);
		glBufferData(type, sizeFunc() * sizeof(T), data, GL_STATIC_DRAW);
		glBindBuffer(type, 0);
	}

	Buffer(GLenum t, vector<T> array):
			Buffer(t, array.data(), [array]() -> GLsizeiptr { return array.size(); }) {}

	virtual ~Buffer() {
		glDeleteBuffers(1, &location);
	}

	void update() {
		glBindBuffer(type, location);
		glBufferData(type, sizeFunc() * sizeof(T), data, GL_DYNAMIC_DRAW);
		glBindBuffer(type, 0);
	}

	void bind(GLuint bindingPoint) {
		glBindBufferBase(type, bindingPoint, location); //bindBufferRange...
	}
};

} /* namespace std */
#endif /* BUFFER_H_ */
