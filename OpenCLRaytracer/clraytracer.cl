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


float random()
{
	unsigned short lfsr = 0xACE1u;
	unsigned bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) & 1);
	return lfsr = (lfsr >> 1) | (bit << 15);
}

float3 nodeIntersection(ray incoming, bool entry, float3 pos, float3 dim)
{ 
	float3 max_pt = (float3)(FLT_MAX, FLT_MAX, FLT_MAX);
	float3 fraction = (float3)(1, 1, 1) / incoming.direction;

	float minx = ((pos.x - (dim.x / 2)) - incoming.origin.x) * fraction.x;
	float maxx = ((pos.x + (dim.x / 2)) - incoming.origin.x) * fraction.x;

	float miny = ((pos.y - (dim.y / 2)) - incoming.origin.y) * fraction.y;
	float maxy = ((pos.y + (dim.y / 2)) - incoming.origin.y) * fraction.y;

	float minz = ((pos.z - (dim.z / 2)) - incoming.origin.z) * fraction.z;
	float maxz = ((pos.z + (dim.z / 2)) - incoming.origin.z) * fraction.z;

	float tmin = max(max(min(minx, maxx), min(miny, maxy)), min(minz, maxz));
	float tmax = min(min(max(minx, maxx), max(miny, maxy)), max(minz, maxz));

	if (tmax < 0)
		return max_pt;
	if (tmin > tmax)
		return max_pt;
	if (entry)
		return incoming.origin + (incoming.direction * tmin);
	return incoming.origin + (incoming.direction * tmax);
}

bool nodeContainsRay(ray incoming, float3 pos, float3 dim)
{
	float3 fraction = (float3)(1, 1, 1) / incoming.direction;
	float minx = ((pos.x - (dim.x / 2)) - incoming.origin.x) * fraction.x;
	float maxx = ((pos.x + (dim.x / 2)) - incoming.origin.x) * fraction.x;
	float miny = ((pos.y - (dim.y / 2)) - incoming.origin.y) * fraction.y;
	float maxy = ((pos.y + (dim.y / 2)) - incoming.origin.y) * fraction.y;
	float minz = ((pos.z - (dim.z / 2)) - incoming.origin.z) * fraction.z;
	float maxz = ((pos.z + (dim.z / 2)) - incoming.origin.z) * fraction.z;
	float tmin = max(max(min(minx, maxx), min(miny, maxy)), min(minz, maxz));
	float tmax = min(min(max(minx, maxx), max(miny, maxy)), max(minz, maxz));
	if (tmax < 0)
		return false;
	if (tmin > tmax)
		return false;
	return true;
}

int getNearestNodes(ray incoming, int offset,
	constant float3 *positions, constant float3 *dimensions, constant char *axis,
	constant int * kdLeft, constant int *kdRight,
	int nodeCount, global int * nodeBuffer)
{
	int node = 0;
	int nodeStack[100] = {};
	int stackCount = 0;
	int leafCount = 1;
	
	while (stackCount > 0 && node < nodeCount && kdLeft[node] < nodeCount && kdRight[node] < nodeCount && leafCount - 1  < nodeCount)
	{
		if (nodeContainsRay(incoming, positions[node], dimensions[node]))
		{
			float3 entry = (float3)nodeIntersection(incoming, true, positions[node], dimensions[node]);
			float3 exit = (float3)nodeIntersection(incoming, false, positions[node], dimensions[node]);
			float a = 0, b = 0, s = 0;
			char ax = axis[node];
			if (ax == 0)
			{
				a = entry.x;
				b = exit.x;
			}
			else if (ax == 1)
			{
				a = entry.y;
				b = exit.y;
			}
			else{
				a = entry.z;
				b = exit.z;
			}
			if (a <= s){
				if (b < s){
					if (kdLeft[node] > 0){
						node = kdLeft[node];
					}
					else{
						nodeBuffer[offset + leafCount - 1] = node;
						leafCount++;
					}
				}
				else if (b == s){
					float v = random();
					if (v < 0.5){
						if (kdRight[node] > 0){
							node = kdRight[node];
						}
						else{
							nodeBuffer[offset + leafCount - 1] = node;
							leafCount++;
						}
					}
					else{
						if (kdLeft[node] > 0){
							node = kdLeft[node];
						}
						else{
							nodeBuffer[offset + leafCount - 1] = node;
							leafCount++;
						}
					}
				}
				else{
					if (kdLeft[node] > 0 && kdRight[node] > 0){
						nodeStack[stackCount] = kdLeft[node]; 
						stackCount++;
						nodeStack[stackCount] = kdRight[node]; 
						stackCount++;
					}
					else{
						nodeBuffer[offset + leafCount - 1] = node;
						leafCount++;
					}
				}
			}
			else{
				if (b > s){
					if (kdRight[node] > 0){
						node = kdRight[node];
					}
					else{
						nodeBuffer[offset + leafCount - 1] = node;
						leafCount++;
					}
				}
				else{
					if (kdLeft[node] > 0 && kdRight[node] > 0){
						nodeStack[stackCount] = kdLeft[node]; 
						stackCount++;
						nodeStack[stackCount] = kdRight[node]; 
						stackCount++;
					}
					else{
						nodeBuffer[offset + leafCount - 1] = node;
						leafCount++;
					}
				}
			}
		}
		else{
			if(stackCount > 0){ 
				node = nodeStack[stackCount];
				stackCount--;
			}
		}
	}
	
	return leafCount;
}

float3 toneMapping(float3 color)
{ 
	float l = 2.0f;
	float3 white = (float3)(l, l, l);
	return (color + (color*color)/(white*white)) / (color + (float3)(1, 1, 1));
}

float3 linearTR(float3 color)
{
	float max_value = 1.5f;
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
	return (float3)(clampf(a.x, 0.0f, 1.0f), clampf(a.y, 0.0f, 1.0f), clampf(a.z, 0.0f, 1.0f));
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

		float fresnal = pow(1.0f - VdotH, 5.0f);
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

	float epsilon = 0.000000001f;
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
	return (float3)((float3)(a *(1.0f - u - v)) + (float3)(b * u) + (float3)(c * v));
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


closest_point closestPointKD(
	ray incoming, constant float3 *ta, constant float3 * tb, constant float3* tc, int triCount,
	constant float3* center, constant float *r, int sprCount,
	constant float3 *positions, constant float3 * dimensions, constant char * axis, constant int * kdLeft, constant int * kdRight,
	constant int * kdTriIndices, constant int * kdTriCount, constant int * kdTriOffset,
	constant int * kdSprIndices, constant int * kdSprCount, constant int * kdSprOffset, int nodeCount, global int * nodeBuffer
	)
{
	float3 closest =  (float3)(FLT_MAX, FLT_MAX, FLT_MAX);
	closest_point closestPt = { closest, -1, -1 };

	int offset = get_global_id(0) * nodeCount;
	int leafCount = getNearestNodes(incoming, offset, positions, dimensions, axis, kdLeft, kdRight, nodeCount, nodeBuffer);
	int nodeIdx = nodeBuffer[offset];
	
	for(int l = 0; l < leafCount; ++l)
	{ 
		int nodeIdx = nodeBuffer[l + offset];
		for (int t = 0; t < kdTriCount[nodeIdx]; ++t){ 
			int i =  kdTriIndices[kdTriOffset[nodeIdx] + t];
			float3 point = (float3)rayIntersectionTri(incoming, (float3)ta[i], (float3)tb[i], (float3)tc[i]);
			if(inBounds(point) && distance(point, incoming.origin) > 0.1f)
			{ 
				if(distance(point, incoming.origin) < distance(closest, incoming.origin))
				{
					closest = point;
					closestPt.point = closest;
					closestPt.object = 0;
					closestPt.index = i;
				}
			}
		}
		for (int s = 0; s < kdSprCount[nodeIdx]; ++s)
		{
			int i = kdSprIndices[kdSprOffset[nodeIdx] + s];
			float3 point = (float3)rayIntersectionSpr(incoming, (float3)center[i], r[i]);
			if (inBounds(point) && distance(point, incoming.origin) > r[i])
			{
				if (distance(point, incoming.origin) < distance(closest, incoming.origin))
				{
					closest = point;
					closestPt.point = point;
					closestPt.object = 1;
					closestPt.index = i;
				}
			}
		}
	}
	
	return closestPt;
}


closest_point closestPoint(
	ray incoming, constant float3* ta, constant float3 * tb, constant float3* tc, int triCount,
	constant float3* center, constant float *r, int sprCount
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
		float3 point = (float3)rayIntersectionSpr(incoming, (float3)center[s], r[s]);
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

bool isInShadow(closest_point closest, int index, int object)
{
	if (inBounds(closest.point))
	{
		if (closest.object != object)
			return true;
		else if (closest.object == object && closest.index != index)
			return true;
	}
	return false;
}

color_data returnColorAtPointOnSphere(
	ray incoming, constant float3* ta, constant float3 * tb, constant float3* tc, constant float* t_kr, constant float* t_kt, int triCount,
	constant float3* center, constant float *r, constant float3* sprColor, constant float* spr_kr, constant float* spr_kt, int sprCount,
	constant float3 *positions, constant float3 * dimensions, constant char * axis, constant int * kdLeft, constant int * kdRight,
	constant int * kdTriIndices, constant int * kdTriCount, constant int * kdTriOffset,
	constant int * kdSprIndices, constant int * kdSprCount, constant int * kdSprOffset, int nodeCount, global int * nodeBuffer,
	constant float3* light, constant float3* light_color, constant int* num_photons, int lightCount, float3 view,
	float3 point, int s, int d, float factor
	)
{ 
	id intersect;
	float3 l_color = (float3)(0, 0, 0);
	int depth = INT_MAX;
	color_data colordata = { l_color, incoming, d, factor };
	for(int l = 0; l < lightCount; ++l)
	{
		for (float radius = 0.25f; radius < 0.75f; radius += 0.1f)
		{
			float3 offset[5] = { (float3)(0, 0, 0), (float3)(radius, 0, 0), (float3)(0, radius, 0), (float3)(-radius, 0, 0), (float3)(0, -radius, 0) };
			for (int o = 0; o < 5; ++o)
			{
				float3 light_pos = light[l] + offset[o];
				ray shadow = { light_pos, (float3)normal(point - light_pos) };
				bool inShadow = false;
				//closest_point closest = closestPoint(shadow, ta, tb, tc, triCount, center, r, sprCount);
				closest_point closest = closestPointKD(shadow, ta, tb, tc, triCount, center, r, sprCount,
					positions, dimensions, axis, kdLeft, kdRight,
					kdTriIndices, kdTriCount, kdTriOffset,
					kdSprIndices, kdSprCount, kdSprOffset,
					nodeCount, nodeBuffer);
				if (!isInShadow(closest, s, 1))
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

					colordata.color += ((float3)cookTorrance(intersect, sprColor[s], 0.1f))* factor / 20.0f;
					colordata.factor /= 2;
					colordata.outray.origin = point;
					depth = d;
					if (spr_kr[s] > 0.0f){
						colordata.factor = factor * spr_kr[s];
						colordata.outray.direction = (float3)reflect(incoming.direction, intersect.normal);
						colordata.outray.direction.z *= -1.0;
					}
					else if (spr_kt[s] > 0.0f){
						colordata.factor = factor * spr_kt[s];
						colordata.outray.direction = (float3)transmit(incoming.direction, intersect.normal);
					}
				}
				else
					break;
			}
		}
	}
	colordata.depth = depth;
	return colordata;
}

color_data returnColorAtPointOnTriangle(
	ray incoming, constant float3* ta, constant float3 * tb, constant float3* tc, constant float* t_kr, constant float* t_kt, int triCount,
	constant float3* center, constant float *r, constant float3* sprColor, constant float* spr_kr, constant float* spr_kt, int sprCount,
	constant float3 *positions, constant float3 * dimensions, constant char * axis, constant int * kdLeft, constant int * kdRight,
	constant int * kdTriIndices, constant int * kdTriCount, constant int * kdTriOffset,
	constant int * kdSprIndices, constant int * kdSprCount, constant int * kdSprOffset, int nodeCount, global int * nodeBuffer,
	constant float3* light, constant float3* light_color, constant int* num_photons, int lightCount, float3 view,
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
		for (float radius = 0.25f; radius < 0.75f; radius += 0.1f)
		{
			float3 offset[5] = { (float3)(0, 0, 0), (float3)(radius, 0, 0), (float3)(0, radius, 0), (float3)(-radius, 0, 0), (float3)(0, -radius, 0) };
			for (int o = 0; o < 5; ++o)
			{
				float3 light_pos = light[l] + offset[o];
				ray shadow = { light_pos, (float3)normal(point - light_pos) };
				bool inShadow = false; 
				closest_point closest = closestPointKD(shadow, ta, tb, tc, triCount, center, r, sprCount,
					positions, dimensions, axis, kdLeft, kdRight,
					kdTriIndices, kdTriCount, kdTriOffset,
					kdSprIndices, kdSprCount, kdSprOffset,
					nodeCount, nodeBuffer);

				if (!isInShadow(closest, t, 0))
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
					colordata.color += ((float3)cookTorrance(intersect, checker_color, 0.5f)) * factor / 20.0f;


					depth = INT_MAX;
					colordata.factor *= 0.5f;
					colordata.outray.origin = point;
					if (t_kr[t] > 0.0f)
					{
						colordata.outray.direction = (float3)reflect(incoming.direction, intersect.normal);
						colordata.outray.direction.z *= -1.0f;
						colordata.factor = factor * t_kr[t];
						depth = d;
					}
					else if (t_kt[t] > 0.0f)
					{
						colordata.outray.direction = (float3)transmit(incoming.direction, intersect.normal);
						colordata.factor = factor * t_kt[t];
						depth = d;
					}
				}
				else{
					depth = INT_MAX;
				}
			}
		}
	}
	colordata.depth = depth;
	return colordata;
}

float3 photonMap(float3 point, global float3* photon_pos, global float3* photon_pow, int total_photons)
{
	float3 color = (float3)(0, 0, 0); 
	const int max_photons = 100;
	float3 photon_colors[max_photons];
	float photon_dis[max_photons];
	int photon_count = 0;
	for (int p = 0; p < total_photons; ++p)
	{
		float current_distance = distance(photon_pos[p], point);
		if (current_distance < 1)
		{
			if (photon_count < max_photons)
			{
				if (photon_count == 0)
				{
					photon_dis[photon_count] = current_distance;
					photon_colors[photon_count] = photon_pow[p];
					photon_count++;
				}
				else{
					int i = 0;
					for (i; i < photon_count; ++i)
					{
						if (photon_dis[i] > current_distance)
						{
							break;
						}
					}
					for (int j = photon_count; j > i; --j)
					{
						photon_dis[j] = photon_dis[j - 1];
						photon_colors[j] = photon_colors[j - 1];
					}
					photon_dis[i] = current_distance;
					photon_colors[i] = photon_pow[p];
					photon_count++;
					photon_count++;
				}
			}
			else
			{
				int i = 0;
				for (i; i < photon_count; ++i)
				{
					if (photon_dis[i] > current_distance)
					{
						break;
					}
				}
				for (int j = photon_count - 1; j > i; --j)
				{
					photon_dis[j] = photon_dis[j - 1];
					photon_colors[j] = photon_colors[j - 1];
				}
				photon_dis[i] = current_distance;
				photon_colors[i] = photon_pow[p];
			}

		}
	}

	for (int p = 0; p < photon_count; p++)
	{
		color += photon_colors[p];
	}
	return color;

}

float3 spawn(
	ray incoming, constant float3* ta, constant float3 * tb, constant float3* tc, constant float* t_kr, constant float* t_kt, int triCount,
	constant float3* center, constant float *r, constant float3* sprColor, constant float* spr_kr, constant float* spr_kt, int sprCount,
	constant float3 *positions, constant float3 * dimensions, constant char * axis, constant int * kdLeft, constant int * kdRight,
	constant int * kdTriIndices, constant int * kdTriCount, constant int * kdTriOffset,
	constant int * kdSprIndices, constant int * kdSprCount, constant int * kdSprOffset, int nodeCount, global int * nodeBuffer,
	constant float3* light, constant float3* lightColor, constant int* num_photons, int lightCount, float3 view,
	global float3* photon_pos, global float3* photon_pow, int total_photons
	)
{
	float3 color = (float3)(0, 0, 0);
	float3 max_pt = (float3)(FLT_MAX, FLT_MAX, FLT_MAX);
	float factor = 1.0;
	ray outray = incoming;
	float3 photon_color = (float3)(0, 0, 0);
	for (int d = 0; d < 3; ++d)
	{
		//closest_point closest = closestPoint(outray, ta, tb, tc, triCount, center, r, sprCount);
			closest_point closest = closestPointKD(outray, ta, tb, tc, triCount, center, r, sprCount,
			positions, dimensions, axis, kdLeft, kdRight, 
			kdTriIndices, kdTriCount, kdTriOffset,
			kdSprIndices, kdSprCount, kdSprOffset,
			nodeCount, nodeBuffer);

		color_data colordata = { (float3)(0, 0, 0), outray, d, factor };
		if (distance(max_pt, closest.point) == 0)
		{
			color += (float3)(0.0f, 0.5f, 1.0f) * factor;
			//break;
		}
		else if (closest.object == 0)
			colordata = returnColorAtPointOnTriangle(outray, ta, tb, tc, t_kr, t_kt, triCount, center, r, sprColor, spr_kr, spr_kt, sprCount,
			positions, dimensions, axis, kdLeft, kdRight,
			kdTriIndices, kdTriCount, kdTriOffset,
			kdSprIndices, kdSprCount, kdSprOffset,
			nodeCount, nodeBuffer, 
			light, lightColor, num_photons, lightCount, view, closest.point, closest.index, d, factor);
		else if (closest.object == 1)
			colordata = returnColorAtPointOnSphere(outray, ta, tb, tc, t_kr, t_kt, triCount, center, r, sprColor, spr_kr, spr_kt, sprCount, 
			positions, dimensions, axis, kdLeft, kdRight,
			kdTriIndices, kdTriCount, kdTriOffset,
			kdSprIndices, kdSprCount, kdSprOffset,
			nodeCount, nodeBuffer,
			light, lightColor, num_photons, lightCount, view, closest.point, closest.index, d, factor);
		
		photon_color += (float3)photonMap(closest.point, photon_pos, photon_pow, total_photons);
		d = colordata.depth;
		factor = colordata.factor;
		outray = colordata.outray;
		color += colordata.color;
	}
	return color;// +(photon_color * 10);
}

float3 clampf3(float3 incoming, float minf, float maxf)
{ 
	return (float3)(max(min(incoming.x, maxf), minf), max(min(incoming.y, maxf), minf), max(min(incoming.z, maxf), minf));
}


float3 random3()
{
	return (float3)(random(), random(), random());
}

__kernel void buildPhotonMap(
	__constant float3 *ta, __constant float3 *tb, __constant float3 *tc, __constant float3 *triColor, __constant float *tri_kr, __constant float *tri_kt, int triCount,
	__constant float3 *center, __constant float *r, __constant float3 *sprColor, __constant float *spr_kr, __constant float *spr_kt, int sprCount,
	__constant float3 *light, __constant float3 *light_color, __constant int *num_photons, int lightCount,
	__global float3 *photon_pos, __global float3 *photon_pow
	)
{
	int total_photons = 0;
	for(int l = 0; l < lightCount; ++l)
	{ 
		int photon_count = 0;
		while(photon_count < num_photons[l])
		{
			photon_count++;
			float3 direction = (float3)(1, 1, 1);// = (float3)normal((float3)random3());
			while ((direction.x * direction.x) + (direction.y * direction.y) + (direction.z * direction.z)> 1)
			{ 
				direction = (float3)normal((float3)random3() * (float3)(0, -1 , 0));
			}
			ray light_ray = { light[l], direction };
			bool stored = false;
			while (!stored)
			{
				closest_point closest = closestPoint(light_ray, ta, tb, tc, triCount, center, r, sprCount);
				float factor = absf(distance(closest.point, light[l]));
				factor *= factor ;
				if (!inBounds(closest.point))
					stored = true;

				int i = closest.index;
				float3 V = (float3)(light[l] - closest.point);
				light_ray.origin = closest.point;
				float Xi = random();
				if(Xi < 0.1)
				{
					photon_pos[total_photons] = closest.point;
					photon_pow[total_photons] = (float3)clampf3((light_color[l]) / (factor), 0, 1);
					photon_count++;
					total_photons++;
					stored = true;
				}
				else{
					if (closest.object == 0)
					{
						float3 N = (float3)triangleNormal(ta[i], tb[i], tc[i]);
						if (tri_kr[i] > 0.3)
						{
							light_ray.direction = (float3)reflect(V, N);
							light_ray.direction.z *= -1;
						}
						else if (tri_kt[i] > 0.3)
						{
							light_ray.direction = (float3)transmit(V, N);
						}
						else
						{
							photon_pos[total_photons] = closest.point;
							photon_pow[total_photons] = (float3)clampf3((light_color[l]) / (factor), 0, 1);
							photon_count++;
							total_photons++;
							stored = true;
						}
					}
					else if (closest.object == 1)
					{
						float3 N = (float3)normal(center[i] - closest.point);
						if (spr_kr[i] > 0.3)
						{
							light_ray.direction = (float3)reflect(V, N);
							light_ray.direction.z *= -1;
						}
						else if (spr_kt[i] > 0.3)
						{
							light_ray.direction = (float3)transmit(V, N);
						}
						else
						{
							photon_pos[total_photons] = closest.point;
							photon_pow[total_photons] = (float3)clampf3((light_color[l]) / (factor), 0, 1);
							photon_count++;
							total_photons++;
							stored = true;
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < total_photons; ++i)
	{
		photon_pow[i] /= total_photons;
	}
	
}

__kernel void scan(
	float3 origin, float3 view, float3 up,
	int W, int H, float w, float h, float f,
	__constant float3 *ta, __constant float3 *tb, __constant float3 *tc, __constant float3 *triColor, __constant float *tri_kr, __constant float *tri_kt, int triCount,
	__constant float3 *center, __constant float *r, __constant float3 *sprColor, __constant float *spr_kr, __constant float *spr_kt, int sprCount,
	__constant float3 *light, __constant float3 *light_color, __constant int *num_photons, int lightCount,
	__global float3 *photon_pos, __global float3* photon_pow, int total_photons,
	__constant float3 *positions, __constant float3 *dimensions, __constant char *axis, __constant int * kdLeft, __constant int *kdRight,
	__constant int *kdTriIndices, __constant int *kdTriCount, __constant int *kdTriOffset,
	__constant int *kdSprIndices, __constant int *kdSprCount, __constant int *kdSprOffset, int nodeCount, __global int * nodeBuffer,
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

	float3 direction = (float3)normal(p_location - origin);
	ray outray = { origin, direction };
	

	float3 final_color = (float3)spawn(outray, ta, tb, tc, tri_kr, tri_kt, triCount, 
		center, r, sprColor, spr_kr, spr_kt, sprCount, 
		positions, dimensions, axis, kdLeft, kdRight,
		kdTriIndices, kdTriCount, kdTriOffset,
		kdSprIndices, kdSprCount, kdSprOffset,
		nodeCount, nodeBuffer,
		light, light_color, num_photons, lightCount, view,
		photon_pos, photon_pow, total_photons);

	color[pixel] = (float3)saturate((float3)toneMapping(final_color));
}
