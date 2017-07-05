#ifndef HRTF_H
#define HRTF_H

#include <cstdint>
#include <string>
#include <vector>

struct Vector3D {
    float x, y, z;
    Vector3D();
    Vector3D(const Vector3D &rhs);
    Vector3D(float x, float y, float z);
    Vector3D& operator = (const Vector3D &rhs);
    float norm() const;
    const Vector3D normalized() const;
    float dot(const Vector3D &rhs) const;
    const Vector3D cross(const Vector3D &rhs) const;
    const Vector3D operator + (const Vector3D &rhs) const;
    const Vector3D operator - (const Vector3D &rhs) const;
    const Vector3D operator * (float rhs) const;
    const Vector3D operator / (float rhs) const;
    Vector3D& operator += (const Vector3D &rhs);
    Vector3D& operator -= (const Vector3D &rhs);
    Vector3D& operator *= (float rhs);
    Vector3D& operator /= (float rhs);
    float angle(const Vector3D &rhs) const;
    float angleDegree(const Vector3D &rhs) const;
    const std :: string toString() const;
};

class HRTF {
private:
    static const int hrtfDataLength = 512;
    std :: vector<int16_t> hrtfData[14][360];
    bool hasHRTFData[14][360];
    Vector3D src, dest, face, up;
    mutable std :: vector<int16_t> midHistory;
public:
    HRTF(std :: string dataPath);
    void setListenerPosition(float x, float y, float z);
    void setSpeakerPosition(float x, float y, float z);
    void setFaceVector(float x, float y, float z);
    void setUpVector(float x, float y, float z);
    void clearHistory() const;
    void DSP(std :: vector<int16_t> &l, std :: vector<int16_t> &r) const;
};

#endif
