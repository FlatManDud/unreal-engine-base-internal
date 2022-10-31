#include <iostream>
#include <Windows.h>
#include "offsets.h"
#include "read_write.h"
#include "spoof.h"
#include <vector>
#include <d3d9types.h>
DWORD base;
DWORD Uworld;
DWORD Gameinstance;
DWORD LocalPawn;
DWORD Localplayer;
DWORD PlayerController;
DWORD Persistentlevel;
DWORD ActorCount;
DWORD AActors;
DWORD RootComp;
int localplayerID;
static int hitbox; // need ids possible : 66 head | 65 neck | 7 chest | 2 pelvis
int Width; // get it your way
int Height;// get it your way

//universal
typedef struct _FNlEntity
{
    uint64_t Actor;
    int ID;
    uint64_t mesh;
}FNlEntity;



std::vector<FNlEntity> entityList;


void Cache()
{
    while (true)
    {
        //could be improved easily
        //should do sigscan instead
        std::vector<FNlEntity> tmpList;

        base = (DWORD)GetModuleHandle(NULL);
        Uworld = read<DWORD>(offsets::uworld);
        Gameinstance = read<DWORD>(Uworld + offsets::gameinstance);
        Localplayer = read<DWORD>(Gameinstance + offsets::localplayer);
        PlayerController = read<DWORD>(Localplayer + offsets::localplayer);
        LocalPawn = read<DWORD>(PlayerController + offsets::playercontroller);
        Persistentlevel = read<DWORD>(Uworld + offsets::persistentlevel);
        ActorCount = read<DWORD>(Persistentlevel + offsets::actorcount);
        AActors = read<DWORD>(Persistentlevel + offsets::aactors);
        RootComp = read<DWORD>(LocalPawn + offsets::rootcomp);

        if (LocalPawn != 0) {
            localplayerID = read<int>(LocalPawn + 0x18);
        }

        for (int i = 0; i < ActorCount; i++)
        {
            uint64_t CurrentActor = read<uint64_t>(AActors + i * 0x8);
            int curactorid = read<int>(CurrentActor + 0x18);
            if (curactorid == localplayerID || curactorid == localplayerID + 765)
            {
                FNlEntity fnlEntity{ };
                fnlEntity.Actor = CurrentActor;
                fnlEntity.mesh = read<uint64_t>(CurrentActor + 0x280);
                fnlEntity.ID = curactorid;
                tmpList.push_back(fnlEntity);
            }
        }
        entityList = tmpList;
        Sleep(1);
    }
}
class Vector3
{
public:
    Vector3() : x(0.f), y(0.f), z(0.f)
    {

    }

    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
    {

    }
    ~Vector3()
    {

    }

    float x;
    float y;
    float z;

    inline float Dot(Vector3 v)
    {
        return x * v.x + y * v.y + z * v.z;
    }

    inline float Distance(Vector3 v)
    {
        return float(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)));
    }

    Vector3 operator+(Vector3 v)
    {
        return Vector3(x + v.x, y + v.y, z + v.z);
    }

    Vector3 operator-(Vector3 v)
    {
        return Vector3(x - v.x, y - v.y, z - v.z);
    }

    Vector3 operator*(float number) const {
        return Vector3(x * number, y * number, z * number);
    }
};

struct FQuat
{
    float x;
    float y;
    float z;
    float w;
};
struct FTransform
{
    FQuat rot;
    Vector3 translation;
    char pad[4];
    Vector3 scale;
    char pad1[4];
    D3DMATRIX ToMatrixWithScale()
    {
        D3DMATRIX m;
        m._41 = translation.x;
        m._42 = translation.y;
        m._43 = translation.z;

        float x2 = rot.x + rot.x;
        float y2 = rot.y + rot.y;
        float z2 = rot.z + rot.z;

        float xx2 = rot.x * x2;
        float yy2 = rot.y * y2;
        float zz2 = rot.z * z2;
        m._11 = (1.0f - (yy2 + zz2)) * scale.x;
        m._22 = (1.0f - (xx2 + zz2)) * scale.y;
        m._33 = (1.0f - (xx2 + yy2)) * scale.z;

        float yz2 = rot.y * z2;
        float wx2 = rot.w * x2;
        m._32 = (yz2 - wx2) * scale.z;
        m._23 = (yz2 + wx2) * scale.y;

        float xy2 = rot.x * y2;
        float wz2 = rot.w * z2;
        m._21 = (xy2 - wz2) * scale.y;
        m._12 = (xy2 + wz2) * scale.x;

        float xz2 = rot.x * z2;
        float wy2 = rot.w * y2;
        m._31 = (xz2 + wy2) * scale.z;
        m._13 = (xz2 - wy2) * scale.x;

        m._14 = 0.0f;
        m._24 = 0.0f;
        m._34 = 0.0f;
        m._44 = 1.0f;

        return m;
    }
};
#define M_PI 3.14159265358979323846264338327950288419716939937510
D3DMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
    float radPitch = (rot.x * float(M_PI) / 180.f);
    float radYaw = (rot.y * float(M_PI) / 180.f);
    float radRoll = (rot.z * float(M_PI) / 180.f);

    float SP = sinf(radPitch);
    float CP = cosf(radPitch);
    float SY = sinf(radYaw);
    float CY = cosf(radYaw);
    float SR = sinf(radRoll);
    float CR = cosf(radRoll);

    D3DMATRIX matrix;
    matrix.m[0][0] = CP * CY;
    matrix.m[0][1] = CP * SY;
    matrix.m[0][2] = SP;
    matrix.m[0][3] = 0.f;

    matrix.m[1][0] = SR * SP * CY - CR * SY;
    matrix.m[1][1] = SR * SP * SY + CR * CY;
    matrix.m[1][2] = -SR * CP;
    matrix.m[1][3] = 0.f;

    matrix.m[2][0] = -(CR * SP * CY + SR * SY);
    matrix.m[2][1] = CY * SR - CR * SP * SY;
    matrix.m[2][2] = CR * CP;
    matrix.m[2][3] = 0.f;

    matrix.m[3][0] = origin.x;
    matrix.m[3][1] = origin.y;
    matrix.m[3][2] = origin.z;
    matrix.m[3][3] = 1.f;

    return matrix;
}

Vector3 ProjectWorldToScreen(Vector3 WorldLocation)
{
    Vector3 Screenlocation = Vector3(0, 0, 0);
    Vector3 Camera;

    auto chain69 = read<uintptr_t>(Localplayer + 0xa8);
    uint64_t chain699 = read<uintptr_t>(chain69 + 8);

    Camera.x = read<float>(chain699 + 0x7F8);
    Camera.y = read<float>(RootComp + 0x12C);

    float test = asin(Camera.x);
    float degrees = test * (180.0 / M_PI);
    Camera.x = degrees;

    if (Camera.y < 0)
        Camera.y = 360 + Camera.y;

    D3DMATRIX tempMatrix = Matrix(Camera);
    Vector3 vAxisX, vAxisY, vAxisZ;

    vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
    vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
    vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);



    uint64_t chain = read<uint64_t>(Localplayer + 0x70);
    uint64_t chain1 = read<uint64_t>(chain + 0x98);
    uint64_t chain2 = read<uint64_t>(chain1 + 0x130);

    Vector3 vDelta = WorldLocation - read<Vector3>(chain2 + 0x10);
    Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

    if (vTransformed.z < 1.f)
        vTransformed.z = 1.f;

    float zoom = read<float>(chain699 + 0x590);

    float FovAngle = 80.0f / (zoom / 1.19f);

    float ScreenCenterX = Width / 2.0f;
    float ScreenCenterY = Height / 2.0f;

    Screenlocation.x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;
    Screenlocation.y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(FovAngle * (float)M_PI / 360.f)) / vTransformed.z;

    return Screenlocation;
}
D3DMATRIX MatrixMultiplication(D3DMATRIX pM1, D3DMATRIX pM2)
{
    D3DMATRIX pOut;
    pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
    pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
    pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
    pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
    pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
    pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
    pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
    pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
    pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
    pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
    pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
    pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
    pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
    pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
    pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
    pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

    return pOut;
}
FTransform GetBoneIndex(DWORD mesh, int index)
{
    DWORD_PTR bonearray;
    bonearray = read<DWORD>(mesh + offsets::bonearray);
    if (bonearray == NULL)
    {
        bonearray = read<DWORD>(mesh + offsets::bonearray + 0x10);
    }
    return read<FTransform>(bonearray + (index * 0x30));

}
Vector3 GetBoneWithRotation(DWORD mesh, int id)
{
    FTransform bone = GetBoneIndex(mesh, id);
    FTransform ComponentToWorld = read<FTransform>(mesh + offsets::componenttoworld);
    D3DMATRIX Matrix;

    Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

    return Vector3(Matrix._41, Matrix._42, Matrix._43);
}
void aimbot(float x, float y) //crappy detected aimbot
{
    float ScreenCenterX = (Width / 2);
    float ScreenCenterY = (Height / 2);
    int AimSpeed = 1.0f;
    float TargetX = 0;
    float TargetY = 0;

    if (x != 0)
    {
        if (x > ScreenCenterX)
        {
            TargetX = -(ScreenCenterX - x);
            TargetX /= AimSpeed;
            if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
        }

        if (x < ScreenCenterX)
        {
            TargetX = x - ScreenCenterX;
            TargetX /= AimSpeed;
            if (TargetX + ScreenCenterX < 0) TargetX = 0;
        }
    }

    if (y != 0)
    {
        if (y > ScreenCenterY)
        {
            TargetY = -(ScreenCenterY - y);
            TargetY /= AimSpeed;
            if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
        }

        if (y < ScreenCenterY)
        {
            TargetY = y - ScreenCenterY;
            TargetY /= AimSpeed;
            if (TargetY + ScreenCenterY < 0) TargetY = 0;
        }
    }
    //idk my brain is fried tonight find a better way
    mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(TargetX), static_cast<DWORD>(TargetY), NULL, NULL);

    return;
}
void AimAt(DWORD entity)
{
    uint64_t currentactormesh = read<uint64_t>(entity + offsets::currentactormesh);
    auto rootHead = GetBoneWithRotation(currentactormesh, hitbox);
    Vector3 rootHeadOut = ProjectWorldToScreen(rootHead); //projectworldtoscreen invalid

    if (rootHeadOut.y != 0 || rootHeadOut.y != 0)
    {
        aimbot(rootHeadOut.x, rootHeadOut.y);
    }
}



DWORD WINAPI Entry(HMODULE hMod)
{
    MessageBoxA(0, "Injected!", "Message", MB_ICONINFORMATION);
    HANDLE handle = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(Cache), nullptr, NULL, nullptr);
    CloseHandle(handle);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        SpoofThreadLocation(Entry, hModule);// if u want
     //   CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Entry, hModule, 0, 0);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

