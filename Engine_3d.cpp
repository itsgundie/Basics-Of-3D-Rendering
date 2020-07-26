// Engine_3d.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//


#include "engine.h"
using namespace std;



struct vec3d
{
	float x, y, z;
};

struct triangle
{
	vec3d p[3];
};

struct mesh
{
	vector<triangle> tris;
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
	};

public:
	bool OnUserCreate() override
	{
		meshCube.tris =
		{
			// SOUTH
			{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

			// EAST                                                      
			{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

			// NORTH                                                     
			{ 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

			// WEST                                                      
			{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

			// TOP                                                       
			{ 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

			// BOTTOM                                                    
			{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f }

		};

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

			triTranslated.p[0].z = triRotatedXZ.p[0].z + 2.0f;
			triTranslated.p[1].z = triRotatedXZ.p[1].z + 2.0f;
			triTranslated.p[2].z = triRotatedXZ.p[2].z + 2.0f;


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

			float lenNorm = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.x);
			normal.x /= lenNorm;
			normal.y /= lenNorm;
			normal.z /= lenNorm;

			if (normal.z < 0)
			{


				MultMatVec(triTranslated.p[0], triProjected.p[0], matProj);
				MultMatVec(triTranslated.p[1], triProjected.p[1], matProj);
				MultMatVec(triTranslated.p[2], triProjected.p[2], matProj);

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



				DrawTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y,
					triProjected.p[2].x, triProjected.p[2].y, PIXEL_SOLID, FG_WHITE);
			}
		};
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
