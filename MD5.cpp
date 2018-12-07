#include "MD5.hpp"
#include <vector>
#include <iostream>
#include <cstring>
#include <string>
#include <cmath>
using std::vector;
using std::string;

//存放填充完成后的原始消息
//每个分组512位，即16个32位的字
vector<vector<bit32> > groups;

//128位的MD5缓冲区
//表示为4个32位寄存器
bit32 CV[4] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476};
//缓冲区初始向量IV
bit32 IV[4] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476};

//加密结果，长度为16的字符串
byte MD5[16];


//填充函数
//输入为字符串类型的原始消息和消息长度
void padding(byte* input, int length) {
    //由于输入的每一个字符占一个字节(8位)
    //因此按字节填充，而不是按位填充
    //每一组64字节，最后一组的8个字节另外填

    //count表示要填充的字节数
    int count = length % 64;
    if (count < 56) count = 64 - 8 - count;
    else count = 128 - 8 - count;

    //填充操作
    byte* paddingInput = new byte[length + count + 8];
    memcpy(paddingInput, input, length);
    paddingInput[length] = 0x80;
    for (int i = length + 1; i < length + count - 8; ++i) {
        paddingInput[i] = 0x00;
    }
    //再向填充好的消息尾部附加消息长度的低8个字节
    //int转byte要注意小端存储
    for (int i = 0; i < 8; ++i) {
        //用((length * 8) >> (i * 8)) & 0xff出现了奇怪的结果
        //改用除法
        paddingInput[length + count + i] = ((byte)((length * 8) / pow(2, (i * 8)))) & 0xff;
    }

    /* std::cout << count + 8 << std::endl;
    for (int i = 0; i < length + count + 8; ++i)
        std::cout << (int)paddingInput[i] << " "; 
    std::cout << std::endl; */
    
    vector<bit32> group;
    group.reserve(16);
    for (int i = 0, flag = 0; i < length + count + 8; i += 4) {
        bit32 word = 0;
        //4个byte转成一个int，采用小端存储
        //如0x01,0x02,0x03,0x04
        //int为0x04030201
        for (int j = 0; j < 4; ++j) {
            word |= (paddingInput[i + j] << (j * 8));
        }
        group.push_back(word);
        flag++;
        if (flag == 16) {
            groups.push_back(group);
            flag = 0;
        }
    }
}

//压缩函数
//对每一个512位(16个32位字)的分组进行压缩
//上一轮压缩的128位结果作为下一轮的CV输入
//最终的结果存放在缓冲区中
void HMD5() {
    for (int i = 0; i < groups.size(); ++i) {
        bit32 A = CV[0];
        bit32 B = CV[1];
        bit32 C = CV[2];
        bit32 D = CV[3];
        //作4轮循环，每一轮作16次迭代
        for (int j = 0; j < 4; ++j) {
            for (int q = 0; q < 16; ++q) {
                bit32 a = A, b = B, c = C, d = D;
                bit32 g;
                int k;
                switch (j) {
                    case 0:
                        g = F(b, c, d);
                        k = q;
                        break;
                    case 1:
                        g = G(b, c, d);
                        k = (1 + 5 * q) % 16;
                        break;
                    case 2:
                        g = H(b, c, d);
                        k = (5 + 3 * q) % 16;
                        break;
                    case 3:
                        g = I(b, c, d);
                        k = (7 * q) % 16;
                        break;
                }

                bit32 tmp = a + g + groups[i][k] + T[j * 16 + q];
                a = b + shiftLeft(tmp, s[j * 16 + q]);
                
                //缓冲区作循环轮换
                //(B, C, D, A) <- (A, B, C, D)
                A = d;
                B = a;
                C = b;
                D = c;
            }
        }
        CV[0] += A;
        CV[1] += B;
        CV[2] += C;
        CV[3] += D;
    }
}

//把128位(4个32位字)的结果转成16个字节的输出
void getBytes() {
    for (int i = 0; i < 4; ++i) {
        //利用了小端编码
        MD5[i * 4 + 0] = CV[i] & 0xff;
        MD5[i * 4 + 1] = (CV[i] >> 8) & 0xff;
        MD5[i * 4 + 2] = (CV[i] >> 16) & 0xff;
        MD5[i * 4 + 3] = (CV[i] >> 24) & 0xff;
    }
}

int main() {
    string test;
    std::cin >> test;
    padding((byte*)test.c_str(), test.length());
    HMD5();
    getBytes();
    // 输出十六进制字符串
    string str = "";
    char HEX[16] = {
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'a', 'b',
        'c', 'd', 'e', 'f'
    };
    for (int i = 0; i < 16; ++i) {
        byte quotient = MD5[i] / 16;
        byte remainder = MD5[i] % 16;
        str.append(1, HEX[quotient]);
        str.append(1, HEX[remainder]);
    }
    std::cout << str << std::endl;
}