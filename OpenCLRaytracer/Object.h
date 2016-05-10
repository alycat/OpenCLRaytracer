#ifndef _OBJECT_H
#define _OBJECT_H

#include <CL\cl.h>

class Object{
public:
	Object();
	Object(cl_float kt, cl_float kr, cl_float roughness, cl_bool checker);
	Object(const Object& copy);
	Object& operator=(const Object& copy);

	~Object();

	cl_float getKt();
	void setKt(cl_float kt);
	__declspec(property(get = getKt, put = setKt)) cl_float kt;

	cl_float getKr();
	void setKr(cl_float kr);
	__declspec(property(get = getKr, put = setKr)) cl_float kr;

	cl_float getRoughness();
	void setRoughness(cl_float roughness);
	__declspec(property(get = getRoughness, put = setRoughness)) cl_float roughness;

	cl_bool getIsChecker();
	void setIsChecker(cl_bool isChecker);
	__declspec(property(get = getIsChecker, put = setIsChecker)) cl_bool isChecker;
protected:
	cl_float m_kt;
	cl_float m_kr;
	cl_float m_roughness;
	cl_bool m_isChecker;
private:
};
#endif