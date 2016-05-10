typedef struct Ray{
	float3 origin;
	float3 direction;
}ray;

typedef struct KDNode{
	float3 dimensions;
	float3 position;
	int num_photons;
	int axis;
	int left;
	int right;
}node;

typedef struct PhotonStruct{
	float3 position;
	float3 power;
	char phi;
	char theta;
	short flag;
}photon;

typedef struct PointOnObject{
	float3 point;
	int object;
	int index;
}closest_point;

typedef struct IntersectData{ 
	float3 point;
	float3 normal;
	float3 source;
	float3 outgoing;
	float3 reflect;
	float3 light_pos;
	float3 light_color;
	int num_photons;
	float3 camera;
	float3 center;
}id;

typedef struct ColorData{
	float3 color;
	ray outray;
	int depth;
	float factor;
}color_data;

float3 toneMapping(float3 color)
{ 
	float l = 2.5f;
	float3 white = (float3)(l, l, l);
	return (color + (color*color)/(white*white)) / (color + (float3)(1, 1, 1));
}

float3 linearTR(float3 color)
{
	float max_value = 5.0f;
	return pow(color, 1.5f) / max_value;
}

float3 normal(float3 vec)
{ 
	float unit = sqrt(dot(vec, vec));
	return (float3)(vec / unit);
}

float3 reflect(float3 V, float3 N)
{
	return -(float3)normal(N - (2 * dot(V, N) * V));//normal((-2 * dot(V, N) * N) + V);
}

float3 transmit(float3 I, float3 N)
{ 
	float inc = 1.05f;
	float d = dot(I, N);
	if(d < 0)
	{ 
		N = -N;
		d = dot(N, I);
	}
	float value = 1 - ((inc * inc) * (1 - (d*d)));
	value = value >= 0.0f ? sqrt(value) : 0.0f;
	float3 T = value > 0.0f ? (I - N * d) * inc + N * value : I;
	return (float3)normal(T);
}


float clampf(float a, float b, float c)
{ 
	return min(max(a, b), c);
}

float absf(float a)
{
	return max(a, -a);
}

float3 saturate(float3 a)
{
	return (float3)(clampf(a.x, 0, 1), clampf(a.y, 0, 1), clampf(a.z, 0, 1));
}

float3 multiplyVectors(float3 a, float3 b)
{
	return (float3)(a.x*b.x, a.y*b.y, a.z*b.z);
}

float3 forcePositive(float3 a)
{
	return (float3)(max(a.x, -a.x), max(a.y, - a.y), max(a.z, -a.z));
}


float3 power(float3 a, float e)
{
	return (float3)(pow(a.x, e), pow(a.y, e), pow(a.z, e));
}

float3 checker(float3 point, float4 uv)
{ 
	float3 color = (float3)(1, 0, 0);
	float u = (point.x - uv.x) / (uv.z - uv.x);
	float v = (point.y - uv.y) / (uv.w - uv.y);
	bool row = false;
	bool col = false;
	if(((int)(u * 20) % 20) % 2 == 0){ 
		row = true;
	}
	if(((int)(v * 20) % 20) % 2 == 0){ 
		col = true;
	}
	if(row == col){ 
		color = (float3)(1, 1, 0);
	}
	return color;
}

float3 illuminate(id intersection, float3 color)
{
	float diff = dot(-intersection.source, intersection.normal) * 0.25f;
	float spec = pow(dot(intersection.reflect, intersection.outgoing), 0.9f) * 0.25f;
	return (float3)saturate(((diff * color) + (spec * intersection.light_color)));
	/*
	float3 s = intersection.light_color * intersection.reflect * intersection.outgoing;
	float3 cdiff = color * intersection.light_color * intersection.source * intersection.normal*12.75f;
	float3 spec = (float3)pow(s, 1) * 0.75f;
	cdiff = (float3)forcePositive(cdiff);
	spec = (float3)forcePositive(spec);
	return cdiff + spec;
	*/
}


float3 cookTorrance(id intersection, float3 color, float roughnessValue)
{
	float3 eyeDir = intersection.outgoing;
	float3 N = intersection.normal;
	float3 lightDir = intersection.outgoing;

	float F0 = 0.8f;
	float k = 0.2f;

	float NdotL = max(dot(N, lightDir), 0.0f);
	float specular = 0.0f;

	if(NdotL > 0.0f)
	{
		float3 halfVector = (float3)normal(lightDir + eyeDir);
		float NdotH = max(dot(N, halfVector), 0.0f);
		float NdotV = max(dot(N, eyeDir), 0.0f);

		float VdotH = max(dot(eyeDir, halfVector), 0.0f);
		float mSquared = roughnessValue * roughnessValue;
		
		float NH2 = 2.0f * NdotH;
		float g1 = (NH2 * NdotV) / VdotH;
		float g2 = (NH2 * NdotL) / VdotH;
		float geoAtt = min(1.0f, min(g1, g2));
		float r1 = 1.0f / (4.0f * mSquared * pow(NdotH, 4.0f));
		float r2 = (NdotH * NdotH - 1.0f) / (mSquared * NdotH * NdotH);
		float roughness = r1 * exp(r2);

		float fresnal = pow(1.0f - VdotH, 1.25f);
		fresnal *= (1.0f - F0);
		fresnal += F0;
		specular = (fresnal * geoAtt * roughness) / (NdotV * NdotL * 3.141529f);
	}
	return (float3)saturate((intersection.light_color * NdotL * (k + specular * (1.0f - k))) + (color*0.1f));
}


float3 rayIntersectionTri(ray inray, float3 a, float3 b, float3 c)
{
	float3 point = (float3)(FLT_MAX, FLT_MAX, FLT_MAX);
	
	float3 e_1 = b - a;
	float3 e_2 = c - a;
	float3 p = cross(inray.direction, e_2);
	float det = dot(e_1, p);

	float epsilon = 0.00000001;
	if (det > -epsilon && det < epsilon){
		return point;
	}

	float f = 1.0f / det;

	float3 T = (float3)inray.origin - a;
	float u = dot(T, p) * f;
	if (u < 0 || u > 1){
		return point;
	}

	float3 Q = cross(T, e_1);
	
	float v = dot(inray.direction, Q) * f;
	if (v < 0.0f || u + v > 1.0f){
		return point;
	}

	float t = dot(Q, e_2) * f;
	if (t <= epsilon){
		return point;
	}

	return (float3)((float3)(a *(1 - u - v)) + (float3)(b * u) + (float3)(c * v));
}

float3 rayIntersectionSpr(ray inray, float3 center, float r)
{ 
	float3 point = (float3)(FLT_MAX, FLT_MAX, FLT_MAX);

	float3 v = inray.origin - center;
	float b = dot(inray.direction, v) * 2;
	float c = dot(v, v) - (r * r);
	float disc = (b * b) - (4 * c);

	if (disc < 0){
		return point;
	}

	disc = sqrt(disc);
	float t_1 = (-b + disc) / 2;
	float t_2 = (-b - disc) / 2;
	float t = min(t_1, t_2);

	if (t > 0){
		return (float3)((inray.direction * t) + inray.origin);
	}

	return point;
}

bool inBounds(float3 point)
{
	if (point.x >= FLT_MAX)
		return false;
	if (point.y >= FLT_MAX)
		return false;
	if (point.z >= FLT_MAX)
		return false;
	return true;
}


float3 triangleNormal(float3 ta, float3 tb, float3 tc)
{
	float3 N = cross(ta, tb);
	float unit = sqrt(dot(N, N));
	return (float3)(N/unit);
}


closest_point closestPoint(
	ray incoming, global float3* ta, global float3 * tb, global float3* tc, int triCount,
	global float3* center, global float *r, int sprCount
	)
{
	float3 closest = (float3)(FLT_MAX, FLT_MAX, FLT_MAX);
	closest_point closestPt = { closest, -1, -1 };
	for (int t = 0; t < triCount; ++t)
	{
		float3 point = (float3)rayIntersectionTri(incoming, (float3)ta[t], (float3)tb[t], (float3)tc[t]);
		if (inBounds(point) && distance(point, incoming.origin) > 0.1f)
		{
			if (distance(point, incoming.origin) < distance(closest, incoming.origin))
			{
				closest = point;
				closestPt.point = closest;
				closestPt.object = 0;
				closestPt.index = t;
			}
		}
	}
	for (int s = 0; s < sprCount; ++s)
	{
		float3 point = (float3)rayIntersectionSpr(incoming, (float3)center[s], (float)r[s]);
		if (inBounds(point) && (distance(point, incoming.origin)) > r[s])
		{
			if (distance(point, incoming.origin) < distance(closest, incoming.origin))
			{
				closest = point;
				closestPt.point = closest;
				closestPt.object = 1;
				closestPt.index = s;
			}
		}
	}
	return closestPt;
}



color_data returnColorAtPointOnSphere(
	ray incoming, global float3* ta, global float3 * tb, global float3* tc, global float* t_kr, global float* t_kt, int triCount,
	global float3* center, global float *r, global float3* sprColor, global float* spr_kr, global float* spr_kt, int sprCount,
	global float3* light, global float3* light_color, global int* num_photons, int lightCount, float3 view,
	float3 point, int s, int d, float factor
	)
{ 
	id intersect;
	float3 l_color = (float3)(0, 0, 0);
	int depth = d;
	color_data colordata = { l_color, incoming, d, factor };
	for(int l = 0; l < lightCount; ++l)
	{
		for (float radius = 0.25; radius < 0.3; radius += 0.01)
		{
			float3 offset[5] = { (float3)(0, 0, 0), (float3)(radius, 0, 0), (float3)(0, radius, 0), (float3)(-radius, 0, 0), (float3)(0, -radius, 0) };
			for (int i = 0; i < 5; ++i)
			{
				float3 light_pos = light[l] + offset[i];
				ray shadow = { light_pos, (float3)normal(point - light_pos) };
				bool inShadow = false;
				for (int i = 0; i < triCount; ++i)
				{
					float3 npoint = (float3)rayIntersectionTri(shadow, (float3)ta[i], (float3)tb[i], (float3)tc[i]);
					inShadow = inBounds(npoint) && distance(npoint, shadow.origin) < 0;
				}
				for (int j = 0; j < sprCount; ++j)
				{
					float3 npoint = (float3)rayIntersectionSpr(shadow, (float3)center[j], (float)r[j]);
					inShadow = inShadow || (inBounds(npoint) && s != j);
				}
				if (!inShadow)
				{
					light_pos = light[l];
					intersect.point = point;
					intersect.normal = (float3)normal((float3)(-point + center[s]));
					intersect.source = (float3)normal((float3)(light_pos - point));
					intersect.outgoing = (float3)normal((float3)(point - light_pos));
					intersect.reflect = (float3)reflect(intersect.source, intersect.normal);
					intersect.light_pos = light_pos;
					intersect.light_color = light_color[l];
					intersect.num_photons = num_photons[l];
					intersect.camera = (float3)normal(-view);
					intersect.center = (float3)center[s];

					colordata.color += ((float3)cookTorrance(intersect, sprColor[s], 0.25f))* factor * (1.0f - radius)/20.0f;
					colordata.factor *= 0.5f;
					colordata.outray.origin = point;
					depth = INT_MAX;
					if (spr_kr[s] > 0.0f){
						colordata.factor = spr_kr[s];
						colordata.outray.direction = (float3)reflect(incoming.direction, intersect.normal);
						colordata.outray.direction.z *= -1.0;
						depth = d;
					}
					else if (spr_kt[s] > 0.0f){
						colordata.factor = spr_kt[s];
						colordata.outray.direction = (float3)transmit(incoming.direction, intersect.normal);
						depth = d;
					}
				}
				else
					depth = INT_MAX;
			}
		}
	}
	colordata.depth = depth;
	return colordata;
}

color_data returnColorAtPointOnTriangle(
	ray incoming, global float3* ta, global float3 * tb, global float3* tc, global float* t_kr, global float* t_kt, int triCount,
	global float3* center, global float *r, global float3* sprColor, global float* spr_kr, global float* spr_kt, int sprCount,
	global float3* light, global float3* light_color, global int* num_photons, int lightCount, float3 view,
	float3 point, int t, int d, float factor
	)
{ 
	id intersect;
	float depth = d;
	float3 l_color = (float3)(0, 0, 0);
	color_data colordata = { l_color, incoming, d, factor };
	float4 uv;
	uv.x = min(min(ta[t].x, tb[t].x), tc[t].x);
	uv.z = max(max(ta[t].x, tb[t].x), tc[t].x);
	uv.w = min(min(ta[t].y, tb[t].y), tc[t].y);
	uv.y = max(max(ta[t].y, tb[t].y), tc[t].y);
	for (int l = 0; l < lightCount; ++l)
	{
		for (float radius = 0.25; radius < 0.3; radius += 0.01)
		{
			float3 offset[5] = { (float3)(0, 0, 0), (float3)(radius, 0, 0), (float3)(0, radius, 0), (float3)(-radius, 0, 0), (float3)(0, -radius, 0) };
			for (int i = 0; i < 5; ++i)
			{
				float3 light_pos = light[l] + offset[i];
				ray shadow = { light_pos, (float3)normal(point - light_pos) };
				bool inShadow = false;
				for (int i = 0; i < triCount; ++i)
				{
					float3 npoint = (float3)rayIntersectionTri(shadow, (float3)ta[i], (float3)tb[i], (float3)tc[i]);
					inShadow = (inBounds(npoint) && i != t);
				}
				for (int j = 0; j < sprCount; ++j)
				{
					float3 npoint = (float3)rayIntersectionSpr(shadow, (float3)center[j], (float)r[j]);
					inShadow = inShadow || inBounds(npoint);
				}
				if (!inShadow)
				{
					light_pos = light[l];
					intersect.point = point;
					intersect.normal = (float3)triangleNormal(ta[t], tb[t], tc[t]);
					intersect.source = (float3)normal((float3)(light_pos - point));
					intersect.outgoing = (float3)normal((float3)(point - light_pos));
					intersect.reflect = (float3)normal((float3)reflect(intersect.source, intersect.normal));
					intersect.light_pos = light_pos;
					intersect.light_color = (float3)light_color[l];
					intersect.num_photons = num_photons[l];
					intersect.camera = (float3)normal(-view);
					intersect.center = (float3)((ta[t] + tb[t] + tc[t]) / 3);
					float3 checker_color = (float3)checker(point, uv);
					colordata.color += ((float3)cookTorrance(intersect, checker_color, 0.3f)) * factor * (1.0f - radius)/20.0f;


					depth = INT_MAX;
					colordata.factor *= 0.5f;
					colordata.outray.origin = point;
					if (t_kr[t] > 0.0f)
					{
						colordata.outray.direction = (float3)reflect(incoming.direction, intersect.normal);
						colordata.outray.direction.z *= -1.0;
						colordata.factor = t_kr[t];
						depth = d;
					}
					else if (t_kt[t] > 0.0f)
					{
						colordata.outray.direction = (float3)transmit(incoming.direction, intersect.normal);
						colordata.factor = t_kt[t];
						depth = d;
					}
				}
				else
					depth = INT_MAX;
			}
		}
	}

	return colordata;
}

float3 spawn(
	ray incoming, global float3* ta, global float3 * tb, global float3* tc, global float* t_kr, global float* t_kt, int triCount,
	global float3* center, global float *r, global float3* sprColor, global float* spr_kr, global float* spr_kt, int sprCount,
	global float3* light, global float3* lightColor, global int* num_photons, int lightCount, float3 view
	)
{
	float3 color = (float3)(0, 0, 0);
	float3 max_pt = (float3)(FLT_MAX, FLT_MAX, FLT_MAX);
	float factor = 1.0;
	ray outray = incoming;
	for (int d = 0; d < 1000; ++d)
	{
		closest_point closest = closestPoint(outray, ta, tb, tc, triCount, center, r, sprCount);
		color_data colordata = { (float3)(0, 0, 0), outray, d, factor };
		if (distance(max_pt, closest.point) == 0)
		{
			color += (float3)(0, 0.5, 1) * factor;
			break;
		}
		if (closest.object == 0)
			colordata = returnColorAtPointOnTriangle(outray, ta, tb, tc, t_kr, t_kt, triCount, center, r, sprColor, spr_kr, spr_kt, sprCount, light, lightColor, num_photons, lightCount, view, closest.point, closest.index, d, factor);
		else if (closest.object == 1)
			colordata = returnColorAtPointOnSphere(outray, ta, tb, tc, t_kr, t_kt, triCount, center, r, sprColor, spr_kr, spr_kt, sprCount, light, lightColor, num_photons, lightCount, view, closest.point, closest.index, d, factor);
		d = colordata.depth - 1;
		factor = colordata.factor;
		outray = colordata.outray;
		color += colordata.color;
	}
	return color;
}

float random()
{
	return 0;
}

float3 random3()
{
	return (float3)(random(), random(), random());
}

__kernel void scan(
	float3 origin, float3 view, float3 up,
	int W, int H, float w, float h, float f,
	__global float3 *ta, __global float3 *tb, __global float3 *tc, __global float3 *triColor, __global float *tri_kr, __global float *tri_kt, int triCount,
	__global float3 *center, __global float *r, __global float3 *sprColor, __global float *spr_kr, __global float *spr_kt, int sprCount,
	__global float3 *light, __global float3 *light_color, __global int *num_photons, int lightCount,
	__global float3 *color
	)
{
	int pixel = get_global_id(0);

	float pixelW = w / (float)W;
	float pixelH = h / (float)H;
	float3 topLeft = (float3)((-w + pixelW) / 2.0f, (h - pixelH) / 2.0f, f);
	float3 COI = (float3)(0.0f, 0.0f, f);
	int x = pixel % W;
	int y = pixel / W;

	float p_x = topLeft.x + pixelW*((float)x);
	float p_y = topLeft.y - pixelH*((float)y);
	float3 n = (float3)normal(origin - COI);
	float3 u_vector = (float3)normal(cross(up, n));
	float3 v_vector = cross(n, u_vector);
	float3 C = origin - n * f;
	float3 L = C - u_vector * w / 2.0f - v_vector * h / 2.0f;
	float3 p_location = L + u_vector * x *pixelW + v_vector * y * pixelH;

	//float3 direction = (float3)normal((float3)(p_x, p_y, f) - origin);
	float3 direction = (float3)normal(p_location - origin);
	ray outray = { origin, direction };

	ray reflection_ray = outray;

	float3 final_color = (float3)spawn(outray, ta, tb, tc, tri_kr, tri_kt, triCount, 
		center, r, sprColor, spr_kr, spr_kt, sprCount, 
		light, light_color, num_photons, lightCount, view);

	color[pixel] = (float3)saturate(/*(float3)toneMapping(final_color)*/final_color/2);
}
