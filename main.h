#pragma once
#include "constants.h"
#include "libconfig.h++"
#include <iostream>
#include <iomanip>
#include <thread>
#include <functional>
#include <memory>
#include <mutex>
#include <chrono>
#include <vector>
#include <algorithm>
#include <locale>
#include <codecvt>
#ifdef _WIN32
#include <windows.h>
#endif
#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h" // support for loading levels from the environment variable
#include <ctime>
#include <libconfig.hh>

#include "scene.h"
#include "table.h"
#include "MainWindow.h"
#include "scene.h"
#include "datapipe.h"
#include "axis.h"

// https://i.voenmeh.ru/kafi5/Kam.loc/inform/UTF-8.htm
// convert UTF-8 string to wstring
inline std::wstring utf8_to_wstring(const std::string& str)
{
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
inline std::string wstring_to_utf8(const std::wstring& str)
{
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(str);
}

inline void toUpper(std::string& str)
{
    transform(
        str.begin(), str.end(),
        str.begin(),
        toupper);
}

inline void setRussianConsole()
{
    // for russian console output in spdlog
    setlocale(LC_ALL, "ru_RU.UTF8");
    setlocale(LC_NUMERIC, "C");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
}

float T_RT_CONST = 4;             // ����� �������=����� ���������� [�]
float pulses_per_meter = 10.0f;   // ������� �������� // ��������� �������� �� ����  // ��� Sick = 1000
float R = 3.0f;                   // ������� �������� // ������ ����� [�]  // ���������� �� 
float F_nom = 1380;               // ����������� ������� [��/���]
float Histeresis = 0.01f;         // ����������. ����������� ���������.
float V_transition = 0.8f;        // C������� �������� � ������������ �������.

bool start = false;
float s0 = 0;                     // ����� ������ �������� [�]
float a1 = 0;                     // ������� ���������� ������� ����� ��������      
float af1 = 0;                    // ������� ���������� ����� ���������� � ��������  (�� �������)  
float t1 = 0;                     // ����� �������� [������]
float s1 = 0;                     // �������� ���������� ����� ���������� [�]
// ����������� ���� ���������� ������������ ��� �������� � ������ � ������������� ���������
float s_ = 0;                     // ������ ���������� � [�]
float s0_ = 0;                    // ���������� ��������� ����� [�]
float s1_ = 0;                    // ���������� �������� ����� [�]
bool trap = false;                // ���� ����� �������� (��������/�����������)
bool blocked = false;             // ���� ���������� ������ �� ����� ��������
bool start_pos_is_set = false;    // ��������� �������� ���������
float v_max = 0;                  // �������� ������������ ��������
int current_row = 0;              // ������� �������� �� �������
int target_row = 1;               // ��������, � ������� ��������� �������

int Init_pac = 0;
std::chrono::steady_clock::time_point t_start;                    // ������ ��������� �������
std::chrono::steady_clock::time_point t_finish;                   // ����� ��������� �������
int Reg_start_counter = 50;
int Reg_enc_counter = 0;

//std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
// std::cout << "Elapseed = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[�s]" << std::endl;
void toNextPovestka();