// Engine_3d.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//


#include "engine.h"
#include <fstream>
#include <strstream>
#include <algorithm>



using namespace std;



struct vec3d
{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
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

	vec3d Matrix_MultVec(mat4x4 &mat, vec3d &inputVec)
	{
		vec3d output;
		output.x = inputVec.x * mat.m[0][0] + inputVec.y * mat.m[1][0] + inputVec.z * mat.m[2][0] + mat.m[3][0];
		output.y = inputVec.x * mat.m[0][1] + inputVec.y * mat.m[1][1] + inputVec.z * mat.m[2][1] + mat.m[3][1];
		output.z = inputVec.x * mat.m[0][2] + inputVec.y * mat.m[1][2] + inputVec.z * mat.m[2][2] + mat.m[3][2];
		output.w = inputVec.x * mat.m[0][3] + inputVec.y * mat.m[1][3] + inputVec.z * mat.m[2][3] + mat.m[3][3];

		return(output);
	}

	mat4x4 Matrix_Identity()
	{
		mat4x4 ident = { 0 };
		ident.m[0][0] = 1.0f;
		ident.m[1][1] = 1.0f;
		ident.m[2][2] = 1.0f;
		ident.m[3][3] = 1.0f;
	
		return(ident);
	}

	mat4x4 Matrix_RotationByX(float fAngleRad)
	{
		mat4x4 xRot = { 0 };
		xRot.m[0][0] = 1;
		xRot.m[1][1] = cosf(fAngleRad);
		xRot.m[1][2] = sinf(fAngleRad);
		xRot.m[2][1] = -sinf(fAngleRad);
		xRot.m[2][2] = cosf(fAngleRad);
		xRot.m[3][3] = 1;

		return(xRot);
	}


	mat4x4 Matrix_RotationByZ(float fAngleRad)
	{
		mat4x4 zRot = { 0 };
		zRot.m[0][0] = cosf(fAngleRad);
		zRot.m[0][1] = sinf(fAngleRad);
		zRot.m[1][0] = -sinf(fAngleRad);
		zRot.m[1][1] = cosf(fAngleRad);
		zRot.m[2][2] = 1.0f;
		zRot.m[3][3] = 1.0f;

		return(zRot);
	}

	mat4x4 Matrix_Translation(float x, float y, float z)
	{
		mat4x4 destination = { 0 };
		
		destination.m[0][0] = 1.0f;
		destination.m[1][1] = 1.0f;
		destination.m[2][2] = 1.0f;
		destination.m[3][3] = 1.0f;
		destination.m[3][0] = x;
		destination.m[3][1] = y;
		destination.m[3][2] = z;
		return(destination);
	}

	mat4x4 Matrix_Projection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
	{
		mat4x4 matProj = { 0 };

		float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);

		matProj.m[0][0] = fAspectRatio * fFovRad;
		matProj.m[1][1] = fFovRad;
		matProj.m[2][2] = fFar / (fFar - fNear);
		matProj.m[2][3] = 1.0f;
		matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matProj.m[3][3] = 0.0f;
	
		return(matProj);
	}


	mat4x4 Matrix_MultMatrix(mat4x4& mat1, mat4x4& mat2)
	{
		mat4x4 result = { 0 };

		for(int col = 0; col < 4; col++)
		{ 
			for (int row = 0; row < 4; row++)
			{
				result.m[row][col] = mat1.m[row][0] * mat2.m[0][col] + mat1.m[row][1] * mat2.m[1][col]
									+ mat1.m[row][2] * mat2.m[2][col] + mat1.m[row][3] * mat2.m[3][col];
			}
		}
		return(result);
	}


	vec3d Vec_Add(vec3d& vec1, vec3d& vec2)
	{
		return{ vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z };
	}


	vec3d Vec_Sub(vec3d& vec1, vec3d& vec2)
	{
		return{ vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z };
	}

	vec3d Vec_Mul(vec3d& vec, float ska)
	{
		return{ vec.x * ska, vec.y * ska, vec.z * ska};
	}

	vec3d Vec_Div(vec3d& vec, float ska)
	{
		return{ vec.x / ska, vec.y / ska, vec.z / ska};
	}

	float Vec_DotPro(vec3d& vec1, vec3d& vec2)
	{
		return(vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z);
	}

	float Vec_Len(vec3d& vec)
	{
		return(sqrtf(Vec_DotPro(vec, vec)));
	}

	vec3d Vec_Normalize(vec3d& vec)
	{
		float len = Vec_Len(vec);
		return{ vec.x / len, vec.y / len, vec.z / len };
	}

	vec3d Vec_CrossPro(vec3d& vec1, vec3d& vec2)
	{
		vec3d rezVec;

		rezVec.x = vec1.y * vec2.z - vec1.z * vec2.y;
		rezVec.y = vec1.z * vec2.x - vec1.x * vec2.z;
		rezVec.z = vec1.x * vec2.y - vec1.y * vec2.x;

		return(rezVec);
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
		meshCube.LoadFromObjectFile("teapot.obj");

		matProj = Matrix_Projection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);

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

		xRot = Matrix_RotationByX(fTheta);
		zRot = Matrix_RotationByZ(fTheta);

		mat4x4 matTrans = Matrix_Translation(0.0f, 0.0f, 4.44f);

		mat4x4 matWorld;

		matWorld = Matrix_Identity();
		matWorld = Matrix_MultMatrix(zRot, xRot);
		matWorld = Matrix_MultMatrix(matWorld, matTrans);

		vector<triangle> vecTrianglesToDraw;


		// Draw Them Triangles
		for (auto tri : meshCube.tris)
		{
			triangle triProjected;
			triangle triTransformed;

			triTransformed.p[0] = Matrix_MultVec(matWorld, tri.p[0]);
			triTransformed.p[1] = Matrix_MultVec(matWorld, tri.p[1]);
			triTransformed.p[2] = Matrix_MultVec(matWorld, tri.p[2]);


			vec3d normal, line1, line2;

			
			// Getting sides-vectors
			line1 = Vec_Sub(triTransformed.p[1], triTransformed.p[0]);
			line2 = Vec_Sub(triTransformed.p[2], triTransformed.p[0]);

			//  Do the Cross Product \{0o0}/ to get Normal of the triangle
			normal = Vec_CrossPro(line1, line2);

			// Getting rid of length, we need only direction, so it's == 1
			normal = Vec_Normalize(normal);

			// Cast a ray from triangle to the camera
			vec3d vCameraRay = Vec_Sub(triTransformed.p[0], vCamera);

			// Triangle would be seen only if Camera is aligned with Normal
			if ((Vec_DotPro(normal, vCameraRay)) < 0.0f)
			{
				// Single Direction Light

				vec3d light_dir = { 0.0f, 1.0f, -1.0f };
				light_dir = Vec_Normalize(light_dir);
				
				float bright = max(0.1f, Vec_DotPro(light_dir, normal));
			
				// Get shade of white for the color
				CHAR_INFO c = GetColour(bright);
				triTransformed.col = c.Attributes;
				triTransformed.sym = c.Char.UnicodeChar;

				// Projecting 3D to 2D
				triProjected.p[0] = Matrix_MultVec(matProj, triTransformed.p[0]);
				triProjected.p[1] = Matrix_MultVec(matProj, triTransformed.p[1]);
				triProjected.p[2] = Matrix_MultVec(matProj, triTransformed.p[2]);
				triProjected.col = triTransformed.col;
				triProjected.sym = triTransformed.sym;

				// Normalizing into cartesian space manually
				triProjected.p[0] = Vec_Div(triProjected.p[0], triProjected.p[0].w);
				triProjected.p[1] = Vec_Div(triProjected.p[1], triProjected.p[1].w);
				triProjected.p[2] = Vec_Div(triProjected.p[2], triProjected.p[2].w);
				
				// Offsetting verteces into normalized visible space
				vec3d vOffsetView = { 1,1,0 };
				triProjected.p[0] = Vec_Add(triProjected.p[0], vOffsetView);
				triProjected.p[1] = Vec_Add(triProjected.p[1], vOffsetView);
				triProjected.p[2] = Vec_Add(triProjected.p[2], vOffsetView);
				triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
				triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
				triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

				vecTrianglesToDraw.push_back(triProjected);
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
