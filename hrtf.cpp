#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>
#include <cmath>
#include "hrtf.hpp"

Vector3D::Vector3D() : x(0.0), y(0.0), z(0.0) {}

Vector3D::Vector3D(const Vector3D &rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}

Vector3D::Vector3D(float x, float y, float z) : x(x), y(y), z(z) {}

Vector3D& Vector3D::operator = (const Vector3D &rhs) {
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
    return *this;
}

float Vector3D::norm() const {
    return sqrt(x * x + y * y + z * z);
}

const Vector3D Vector3D::normalized() const {
    float n = norm();
    if (n > 0) {
        return Vector3D(x / n, y / n, z / n);
    } else {
        return Vector3D();
    }
}

float Vector3D::dot(const Vector3D &rhs) const {
    return x * rhs.x + y * rhs.y + z * rhs.z;
}

const Vector3D Vector3D::cross(const Vector3D &rhs) const {
    return Vector3D(y * rhs.z - rhs.y * z, z * rhs.x - rhs.z * x, x * rhs.y - rhs.x * y);
}

const Vector3D Vector3D::operator + (const Vector3D &rhs) const {
    return Vector3D(x + rhs.x, y + rhs.y, z + rhs.z);
}

const Vector3D Vector3D::operator - (const Vector3D &rhs) const {
    return Vector3D(x - rhs.x, y - rhs.y, z - rhs.z);
}

const Vector3D Vector3D::operator * (float rhs) const {
    return Vector3D(x * rhs, y * rhs, z * rhs);
}

const Vector3D Vector3D::operator / (float rhs) const {
    return Vector3D(x / rhs, y / rhs, z / rhs);
}

Vector3D& Vector3D::operator += (const Vector3D &rhs) {
    return *this = *this + rhs;
}

Vector3D& Vector3D::operator -= (const Vector3D &rhs) {
    return *this = *this - rhs;
}

Vector3D& Vector3D::operator *= (float rhs) {
    return *this = *this * rhs;
}

Vector3D& Vector3D::operator /= (float rhs) {
    return *this = *this / rhs;
}

float Vector3D::angle(const Vector3D &rhs) const {
    return acos(dot(rhs) / (norm() * rhs.norm()));
}

float Vector3D::angleDegree(const Vector3D &rhs) const {
    return angle(rhs) * 180 / M_PI;
}

const std :: string Vector3D::toString() const {
    return '(' + std :: to_string(x) + ", " + std :: to_string(y) + ", " + std :: to_string(z) + ')';
}

HRTF::HRTF(std :: string dataPath) :
    src(Vector3D(1.0, 0.0, 0.0)),
    face(Vector3D(0.0, 1.0, 0.0)),
    up(Vector3D(0.0, 0.0, 1.0)) {
    clearHistory();
    if (dataPath == "") {
        dataPath = "./";
    }
    if (dataPath[dataPath.length() - 1] != '/') {
        dataPath += '/';
    }
    for (int i = 0; i < 14; ++i) {
        bool ok = false;
        for (int j = 0; j < 360; ++j) {
            char tmpPath[22] = {};
            int elev = (i - 4) * 10;
            sprintf(tmpPath, "elev%d/L%de%03da.dat", elev, elev, j);
            std :: string filename = dataPath + tmpPath;
            std :: ifstream fs(filename, std :: ios :: binary);
            if (fs) {
                hasHRTFData[i][j] = true;
                ok = true;
                uint8_t tmp[hrtfDataLength << 1];
                fs.read((char*)tmp, hrtfDataLength << 1);
                fs.close();
                for (int k = 0; k < hrtfDataLength << 1; k += 2) {
                    hrtfData[i][j].push_back(int16_t((tmp[k] << 8) | tmp[k + 1]));
                }
            } else {
                hasHRTFData[i][j] = false;
            }
        }
        if (!ok) {
            fprintf(stderr, "Unable to read HRTF data!\n");
            exit(1);
        }
    }
}

void HRTF::setListenerPosition(float x, float y, float z) {
    dest = Vector3D(x, y, z);
}

void HRTF::setSpeakerPosition(float x, float y, float z) {
    src = Vector3D(x, y, z);
}

void HRTF::setFaceVector(float x, float y, float z) {
    face = Vector3D(x, y, z).normalized();
}

void HRTF::setUpVector(float x, float y, float z) {
    up = Vector3D(x, y, z).normalized();
}

void HRTF::clearHistory() const {
    midHistory = std :: vector<int16_t>(hrtfDataLength);
}

void HRTF::DSP(std :: vector<int16_t> &l, std :: vector<int16_t> &r) const {
    Vector3D relative = src - dest;
    if (relative.norm() == 0 || face.norm() == 0 || up.norm() == 0 || l.size() != r.size()) {
        return;
    }
    Vector3D horizontal = relative - up * relative.dot(up);
    Vector3D z = face.cross(up);
    int elevation = int(floor(13.5 - relative.angleDegree(up) / 10));
    if (elevation < 0) {
        elevation = 0;
    }
    int azimuth = int(floor(horizontal.angleDegree(face) + 0.5));
    for (int al = azimuth, ar = azimuth;; al = (al + 180) % 181, ar = (ar + 1) % 181) {
        if (hasHRTFData[elevation][al]) {
            azimuth = al;
            break;
        }
        if (hasHRTFData[elevation][ar]) {
            azimuth = ar;
            break;
        }
    }
    const std :: vector<int16_t> *lHRTFPtr = &hrtfData[elevation][azimuth];
    const std :: vector<int16_t> *rHRTFPtr = &hrtfData[elevation][(360 - azimuth) % 360];
    if (horizontal.dot(z) < 0.0) {
        lHRTFPtr = &hrtfData[elevation][(360 - azimuth) % 360];
        rHRTFPtr = &hrtfData[elevation][azimuth];
    }
    const std :: vector<int16_t> &lHRTF = *lHRTFPtr;
    const std :: vector<int16_t> &rHRTF = *rHRTFPtr;
    std :: vector<int16_t> mid;
    for (int i = 0; i < l.size(); ++i) {
        mid.push_back((l[i] + r[i]) >> 1);
    }
    for (int i = 0; i < mid.size() ; ++i) {
        int64_t tmpL = 0, tmpR = 0;
        for (int j = 0; j < hrtfDataLength; ++j) {
            if (i >= j) {
                tmpL += mid[i - j] * lHRTF[j];
                tmpR += mid[i - j] * rHRTF[j];
            } else {
                tmpL += midHistory[hrtfDataLength + i - j] * lHRTF[j];
                tmpR += midHistory[hrtfDataLength + i - j] * rHRTF[j];
            }
        }
        l[i] = tmpL >> 16;
        r[i] = tmpR >> 16;
        int norm = relative.norm();
        if (norm > 1) {
            l[i] /= norm;
            r[i] /= norm;
        }
    }
    if (mid.size() >= hrtfDataLength) {
        midHistory = std :: vector<int16_t>(mid.cend() - hrtfDataLength, mid.cend());
    } else {
        midHistory = std :: vector<int16_t>(midHistory.cbegin() + mid.size(), midHistory.cend());
        midHistory.insert(midHistory.cend(), mid.cbegin(), mid.cend());
    }
}
