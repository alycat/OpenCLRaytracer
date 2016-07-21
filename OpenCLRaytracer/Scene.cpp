#include "stdafx.h"
#include "Scene.h"
#include "Parser.h"
#include "Global.h"

Scene::Scene()
	:m_lgtMgr(new LightManager(m_bufferMgr)), m_objMgr(new ObjectManager(m_bufferMgr))
{
	static const cl_float3 zero3 = { 0, 0, 0 };
	cl_float3 dimensions = { 10, 10, 10 };
	m_tree = new KDTree(BoundingBox{ zero3, dimensions }, *m_objMgr, m_bufferMgr);
	m_ctxMgr = ContextManager::instance();
	const char* programSource = Parser::readFile("clraytracer.cl");
	cl_int status;
	cl_program program = clCreateProgramWithSource(m_ctxMgr->context(), 1, (const char**)&programSource, NULL, &status);
	clBuildProgram(program, m_ctxMgr->numDevices(), m_ctxMgr->devices(), NULL, NULL, NULL);
	m_kerMgr.createKernel(program, "buildPhotonMap", "photonKernel");
	m_kerMgr.createKernel(program, "scan", "kernel");
}

Scene::Scene(const Scene& copy)
	: m_lgtMgr(copy.m_lgtMgr), m_objMgr(copy.m_objMgr), m_cam(copy.m_cam), m_bufferMgr(m_bufferMgr)
{}

Scene::~Scene()
{
	if (m_lgtMgr)
		delete m_lgtMgr;
	m_lgtMgr = nullptr;
	if (m_objMgr)
		delete m_objMgr;
	m_objMgr = nullptr;
	if (m_tree)
		delete m_tree;
	m_tree = nullptr;
}

Scene& Scene::operator=(const Scene& copy)
{
	m_lgtMgr = copy.m_lgtMgr;
	m_objMgr = copy.m_objMgr;
	m_cam = copy.m_cam;
	m_bufferMgr = copy.m_bufferMgr;
	return *this;
}

void Scene::setCamera(cl_float3 origin, cl_float3 view, cl_float3 up, cl_int W, cl_int H, cl_float w, cl_float h, cl_float f)
{
	m_cam = Camera(origin, view, up, W, H, w, h, f);
}

void Scene::setCamera(cl_float3 origin, cl_float3 view, cl_float3 up)
{
	m_cam.origin = origin;
	m_cam.view = view;
	m_cam.up = up;
}

void Scene::addLight(Light light)
{
	m_lgtMgr->addLight(light);
}

void Scene::addSphere(Sphere sphere)
{
	m_objMgr->addSphere(sphere);
}

void Scene::addTriangle(Triangle triangle)
{
	m_objMgr->addTriangle(triangle);
}

void Scene::setOrigin(cl_float3 origin)
{
	m_cam.setOrigin(origin);
}

void Scene::setView(cl_float3 view)
{
	m_cam.setView(view);
}

void Scene::setUp(cl_float3 up)
{
	m_cam.setUp(up);
}

void Scene::setW(cl_int W)
{
	m_cam.setW(W);
}

void Scene::setH(cl_int H)
{
	m_cam.setH(H);
}

void Scene::setWidth(cl_float w)
{
	m_cam.setWidth(w);
}

void Scene::setHeight(cl_float h)
{
	m_cam.setHeight(h);
}

void Scene::createBuffers()
{
	m_tree->createKDTreeBuffers();
	m_objMgr->createObjectBuffers();
	m_lgtMgr->createLightBuffers();
	m_color = (cl_float3*)malloc(sizeof(cl_float3) * m_cam.W * m_cam.H);
	m_bufferMgr.createBuffer("color", sizeof(cl_float3) * m_cam.W * m_cam.H, m_color, CL_MEM_WRITE_ONLY);
	m_bufferMgr.enqueueWriteBuffer();
}

cl_float3 Scene::color(int x, int y)
{
	auto color_data = m_bufferMgr.data("color");
	cl_float3 color;
	color.x = 0, color.y = 0, color.z = 0;
	return color;
}

cl_float3* Scene::color()
{
	return (cl_float3*)m_bufferMgr.data("color");
}

void Scene::setParamaters()
{
	cl_mem ta = m_bufferMgr.buffer("ta");
	cl_mem tb = m_bufferMgr.buffer("tb");
	cl_mem tc = m_bufferMgr.buffer("tc");
	cl_mem triColor = m_bufferMgr.buffer("triColor");
	cl_mem t_kr = m_bufferMgr.buffer("t_kr");
	cl_mem t_kt = m_bufferMgr.buffer("t_kt");

	cl_mem center = m_bufferMgr.buffer("center");
	cl_mem r = m_bufferMgr.buffer("r");
	cl_mem sprColor = m_bufferMgr.buffer("sprColor");
	cl_mem s_kr = m_bufferMgr.buffer("s_kr");
	cl_mem s_kt = m_bufferMgr.buffer("s_kt");

	cl_mem lightPos = m_bufferMgr.buffer("lightPos");
	cl_mem lightColor = m_bufferMgr.buffer("lightColor");
	cl_mem numPhotons = m_bufferMgr.buffer("numPhotons");

	cl_mem photonPos = m_bufferMgr.buffer("photonPos");
	cl_mem photonPow = m_bufferMgr.buffer("photonPow");

	cl_mem positions = m_bufferMgr.buffer("positions");
	cl_mem dimensions = m_bufferMgr.buffer("dimensions");
	cl_mem axis = m_bufferMgr.buffer("axis");
	cl_mem kdLeft = m_bufferMgr.buffer("kdLeft");
	cl_mem kdRight = m_bufferMgr.buffer("kdRight");

	cl_mem kdTriIndices = m_bufferMgr.buffer("kdTriIndices");
	cl_mem kdTriCount = m_bufferMgr.buffer("kdTriCount");
	cl_mem kdTriOffset = m_bufferMgr.buffer("kdTriOffset");

	cl_mem kdSprIndices = m_bufferMgr.buffer("kdSprIndices");
	cl_mem kdSprCount = m_bufferMgr.buffer("kdSprCount");
	cl_mem kdSprOffset = m_bufferMgr.buffer("kdSprOffset");

	cl_mem color = m_bufferMgr.buffer("color");

	cl_int triCount = m_objMgr->triCount();
	cl_int sprCount = m_objMgr->sprCount();
	cl_int lgtCount = m_lgtMgr->lightCount();

	cl_int nodeCount = m_tree->nodeCount();
	size_t globalWorkSize[1];
	cl_int status;
	cl_float3* triangleA = (cl_float3*)m_bufferMgr.data("ta");
	
	status = m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &ta);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &tb);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &tc);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &triColor);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &t_kr);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &t_kt);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_int), &triCount);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &center);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &r);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &sprColor);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &s_kr);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_int), &s_kt);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_int), &sprCount);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &lightPos);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &lightColor);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &numPhotons);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_int), &lgtCount);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &photonPos);
	status |= m_kerMgr.addArgument("photonKernel", sizeof(cl_mem), &photonPow);

	globalWorkSize[0] = 1;
	cl_kernel kernel = m_kerMgr.kernel("photonKernel");
	status = clEnqueueNDRangeKernel(m_ctxMgr->cmdQueue(), kernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL);
	cl_int W = m_cam.W;
	cl_int H = m_cam.H;
	cl_float w = m_cam.w;
	cl_float h = m_cam.h;
	cl_float f = sqrt(pow(W, 2) + pow(H, 2)) * 9000000.0f;
	cl_int total_photons = m_lgtMgr->totalPhotons();

	clEnqueueReadBuffer(m_ctxMgr->cmdQueue(), photonPos, CL_TRUE, 0, sizeof(cl_float3) * total_photons, (void*)(m_bufferMgr.data("photonPos")), 0, NULL, NULL);
	clEnqueueReadBuffer(m_ctxMgr->cmdQueue(), photonPow, CL_TRUE, 0, sizeof(cl_float3) * total_photons, (void*)(m_bufferMgr.data("photonPow")), 0, NULL, NULL);

	status = m_kerMgr.addArgument("kernel", sizeof(cl_float3), &m_cam.origin);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_float3), &m_cam.view);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_float3), &m_cam.up);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_int), &W);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_int), &H);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_float), &w);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_float), &h);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_float), &f);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &ta);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &tb);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &tc);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &triColor);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &t_kr);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &t_kt);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_int), &triCount);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &center);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &r);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &sprColor);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &s_kr);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &s_kt);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_int), &sprCount);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &lightPos);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &lightColor);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &numPhotons);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_int), &lgtCount);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &photonPos);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &photonPow);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_int), &total_photons);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &positions);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &dimensions);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &axis);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &kdLeft);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &kdRight);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &kdTriIndices);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &kdTriCount);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &kdTriOffset);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &kdSprIndices);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &kdSprCount);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &kdSprOffset);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_int), &nodeCount);
	status |= m_kerMgr.addArgument("kernel", sizeof(cl_mem), &color);

	kernel = m_kerMgr.kernel("kernel");

	globalWorkSize[0] = m_cam.W * m_cam.H;
	status = clEnqueueNDRangeKernel(m_ctxMgr->cmdQueue(), kernel, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL);
	status = clEnqueueReadBuffer(m_ctxMgr->cmdQueue(), color, CL_TRUE, 0, sizeof(cl_float3) * m_cam.W * m_cam.H, (void*)(m_bufferMgr.data("color")), 0, NULL, NULL);
}