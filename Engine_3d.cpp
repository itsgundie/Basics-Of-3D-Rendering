// Engine_3d.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//


#include "engine.h"
#include <fstream>
#include <strstream>
#include <algorithm>



using namespace std;



struct vec3d
{
	float x, y, z;
};

struct triangle
{
	vec3d p[3];
	wchar_t sym;
	short col;
};

struct mesh
{
	vector<triangle> tris;

	bool LoadFromObjectFile(string sFilename)
	{
		ifstream f(sFilename);
		if (!f.is_open())
			return(false);

		//Caching verts for parsing

		vector<vec3d> verts;

		while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);

			strstream s;
			s << line;

			char junk;

			if (line[0] == 'v')
			{
				vec3d vec;
				
				s >> junk >> vec.x >> vec.y >> vec.z;
				verts.push_back(vec);
			}

			if (line[0] == 'f')
			{
				int triangles[3];

				s >> junk >> triangles[0] >> triangles[1] >> triangles[2];
				tris.push_back({ verts[triangles[0] - 1], verts[triangles[1] - 1], verts[triangles[2] - 1] });
			}
		}

		return(true); 
	}
};

struct mat4x4
{
	float m[4][4] = { 0 };
};


class Engine3D : public olcConsoleGameEngine
{
public:
	Engine3D()
	{
		m_sAppName = L"DEMO 3D";
	}

private:
	mesh meshCube;
	mat4x4 matProj;

	vec3d vCamera;

	float fTheta;

	void MultMatVec(vec3d& input, vec3d& output, mat4x4& mat)
	{
		output.x = input.x * mat.m[0][0] + input.y * mat.m[1][0] + input.z * mat.m[2][0] + mat.m[3][0];
		output.y = input.x * mat.m[0][1] + input.y * mat.m[1][1] + input.z * mat.m[2][1] + mat.m[3][1];
		output.z = input.x * mat.m[0][2] + input.y * mat.m[1][2] + input.z * mat.m[2][2] + mat.m[3][2];

		float w = input.x * mat.m[0][3] + input.y * mat.m[1][3] + input.z * mat.m[2][3] + mat.m[3][3];

		if (w != 0.0f)
		{
			output.x /= w;
			output.y /= w;
			output.z /= w;
		}
	}

	CHAR_INFO GetColour(float lum)
	{
		short bg_col, fg_col;
		wchar_t sym;
		int pixel_bw = (int)(13.0f * lum);
		switch (pixel_bw)
		{
		case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID; break;

		case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
		case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
		case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;

		case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
		case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
		case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;

		case 9:  bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
		case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
		case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
		case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;
		default:
			bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
		}

		CHAR_INFO c;
		c.Attributes = bg_col | fg_col;
		c.Char.UnicodeChar = sym;
		return c;
	}

public:
	bool OnUserCreate() override
	{
		//meshCube.tris =
		//{
		//	// SOUTH
		//	{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
		//	{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

		//	// EAST                                                      
		//	{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
		//	{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

		//	// NORTH                                                     
		//	{ 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
		//	{ 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

		//	// WEST                                                      
		//	{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
		//	{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

		//	// TOP                                                       
		//	{ 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
		//	{ 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

		//	// BOTTOM                                                    
		//	{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
		//	{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f }

		//};

		meshCube.LoadFromObjectFile("Spaceship.obj");

		// Hardcoded Projection Matrix

		float fNear = 0.1f;
		float fFar = 1000.0f;
		float fFov = 90.0f;
		float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
		float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

		matProj.m[0][0] = fAspectRatio * fFovRad;
		matProj.m[1][1] = fFovRad;
		matProj.m[2][2] = fFar / (fFar - fNear);
		matProj.m[2][3] = 1.0f;
		matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matProj.m[3][3] = 0.0f;

		return (true);
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		// Clearing The Screen With Black Color
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		// Rotation Matrices by Axis and Angle
		mat4x4 xRot;
		mat4x4 zRot;

		fTheta += 1.0f * fElapsedTime;

		// X-rotation Matrix

		xRot.m[0][0] = 1;
		xRot.m[1][1] = cosf(fTheta * 0.5f);
		xRot.m[1][2] = sinf(fTheta * 0.5f);
		xRot.m[2][1] = -sinf(fTheta * 0.5f);
		xRot.m[2][2] = cosf(fTheta * 0.5f);
		xRot.m[3][3] = 1;

		// Z-rotation Matrix

		zRot.m[0][0] = cosf(fTheta * 0.5f);
		zRot.m[0][1] = sinf(fTheta * 0.5f);
		zRot.m[1][0] = -sinf(fTheta * 0.5f);
		zRot.m[1][1] = cosf(fTheta * 0.5f);
		zRot.m[2][2] = 1;
		zRot.m[3][3] = 1;


		vector<triangle> vecTrianglesToDraw;


		// Draw Them Triangles
		for (auto tri : meshCube.tris)
		{
			triangle triProjected;
			triangle triTranslated;
			triangle triRotatedX;
			triangle triRotatedXZ;

			// Rotate Around X-axis

			MultMatVec(tri.p[0], triRotatedX.p[0], xRot);
			MultMatVec(tri.p[1], triRotatedX.p[1], xRot);
			MultMatVec(tri.p[2], triRotatedX.p[2], xRot);

			// Rotate Result Around Z-axis

			MultMatVec(triRotatedX.p[0], triRotatedXZ.p[0], zRot);
			MultMatVec(triRotatedX.p[1], triRotatedXZ.p[1], zRot);
			MultMatVec(triRotatedX.p[2], triRotatedXZ.p[2], zRot);



			triTranslated = triRotatedXZ;

			triTranslated.p[0].z = triRotatedXZ.p[0].z + 8.0f;
			triTranslated.p[1].z = triRotatedXZ.p[1].z + 8.0f;
			triTranslated.p[2].z = triRotatedXZ.p[2].z + 8.0f;


			vec3d normal, line1, line2;

			line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
			line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
			line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

			line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
			line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
			line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

			normal.x = line1.y * line2.z - line1.z * line2.y;
			normal.y = line1.z * line2.x - line1.x * line2.z;
			normal.z = line1.x * line2.y - line1.y * line2.x;

			float lenNorm = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
			normal.x /= lenNorm;
			normal.y /= lenNorm;
			normal.z /= lenNorm;

			if ((normal.x * (triTranslated.p[0].x - vCamera.x) +
				normal.y * (triTranslated.p[0].y - vCamera.y) +
				normal.z * (triTranslated.p[0].z - vCamera.z)) < 0.0f)
			{
				// Single Direction Light

				vec3d light_dir = { 0.0f, 0.0f, -1.0f };
				float lightLen = sqrtf(light_dir.x * light_dir.x + light_dir.y * light_dir.y + light_dir.z * light_dir.z);
				light_dir.x /= lightLen;
				light_dir.y /= lightLen;
				light_dir.z /= lightLen;

				float dotLight = normal.x * light_dir.x + normal.y * light_dir.y + normal.z * light_dir.z;

				CHAR_INFO c = GetColour(dotLight);
				triTranslated.col = c.Attributes;
				triTranslated.sym = c.Char.UnicodeChar;

				MultMatVec(triTranslated.p[0], triProjected.p[0], matProj);
				MultMatVec(triTranslated.p[1], triProjected.p[1], matProj);
				MultMatVec(triTranslated.p[2], triProjected.p[2], matProj);
				triProjected.col = triTranslated.col;
				triProjected.sym = triTranslated.sym;

				triProjected.p[0].x += 1.0f;
				triProjected.p[0].y += 1.0f;
				triProjected.p[1].x += 1.0f;
				triProjected.p[1].y += 1.0f;
				triProjected.p[2].x += 1.0f;
				triProjected.p[2].y += 1.0f;

				triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
				triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
				triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

				vecTrianglesToDraw.push_back(triProjected);

				//FillTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y,
				//	triProjected.p[2].x, triProjected.p[2].y, triProjected.sym, triProjected.col);

				//DrawTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y,
				//	triProjected.p[2].x, triProjected.p[2].y, PIXEL_SOLID, FG_WHITE);
			}
		}

		// Sort triangles for artist algo, i.e. from back to front

		sort(vecTrianglesToDraw.begin(), vecTrianglesToDraw.end(), [](triangle& t1, triangle& t2)
		{
			float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
			float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
			return(z1 > z2);
		});
		

		for (auto& triProjected : vecTrianglesToDraw)
		{
			FillTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y,
				triProjected.p[2].x, triProjected.p[2].y, triProjected.sym, triProjected.col);

			DrawTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y,
				triProjected.p[2].x, triProjected.p[2].y, PIXEL_SOLID, FG_WHITE);
		}


		return(true);
	}
};

int main()
{
	Engine3D demo;

	if (demo.ConstructConsole(256, 240, 4, 4))
		demo.Start();
	return(0);
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
