#define _CRT_SECURE_NO_WARNINGS // special for Visual studio
#include "bigInt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bigInt::bigInt()
{
	_size = 1;
	_sign = 0;
	_digits = new digit[1];
	_digits[0] = 0;
}

bigInt::bigInt(const char* string)
{// конструктор из строки, в которой записано число в 10-ричной форме
	if (!string)
		return;

	int strSize = strlen(string);
	int strSign = 0;
	if (string[0] == '-')
	{
		strSize--;
		strSign = 1;
	}
	char* strCpy = new char[strlen(string) + 1]; // потому что string - const char
	strcpy(strCpy, string);

	// проверка строки на наличие нечисловых символов
	char* pStr = strCpy + strSign;
	while (*pStr)
	{
		if (*pStr < '0' || *pStr > '9')
		{
			_size = 0;
			delete[] strCpy;
			_setSize(1);
			return;
		}
		pStr++;
	}

	pStr = strCpy + strSign;
	bigInt b1000000000 = 1000000000; // чтобы в цикле не вызывать каждый раз конструктор
	bigInt res;
	// обработка сразу по 9 символов
	for (int i = 0; i < strSize / 9; i++)
	{
		char splStr[10];
		memcpy(splStr, pStr, 9);
		splStr[9] = '\0';
		pStr += 9;
		digit digitI = atol(splStr);
		res = res * b1000000000 + digitI;
	}

	// обработка оставшихся символов
	while (*pStr)
	{
		res = res * 10 + (*pStr - '0');
		pStr++;
	}
	delete[] strCpy;
	res._sign = strSign;
	res._delLeadZeros();
	this->_size = 0;
	*this = res;
}

bigInt::bigInt(const bigInt &rhv)
{
	_copy(rhv);
}

bigInt::bigInt(long long int value)
{
	_digits = new digit[3];
	_size = 0;
	_sign = 0;
	long long int carry = value;
	if (value < 0)
	{
		_sign = 1;
		carry = -value;
	}
	do
	{
		_size++;
		_digits[_size - 1] = carry % BASE;
		carry = carry / BASE;
	} while (carry);
}

bigInt::~bigInt()
{
	if (_size) delete[] _digits;
}


char* bigInt::getString()
{// возвращает строку, в которой записано число в 10-ричной системе счисления

	bigInt thisNumber = *this;
	thisNumber._sign = 0;
	char* strBuffer = new char[MAX_10_LEN]();	// здесь закралось ограничение
												// на максимальную длину числа в 10-чной записи
	char* pString = &(strBuffer[MAX_10_LEN - 1]); // указатель на текущую позицию для записи числа
	bigInt b1000000000 = 1000000000;
	char splStr[10];
	do
	{
		pString -= 9;
		if (pString < strBuffer)
		{// в случае, если число сильно длинное, выведется только младшая часть числа
			pString += 9;
			break;
		}

		bigInt remainder;
		thisNumber = _divividing(thisNumber, b1000000000, remainder);
		sprintf(splStr, "%09u", remainder[0]);
		memcpy(pString, splStr, 9);
		
	} while (thisNumber > (long long int) 0);

	// удаление ведущих нулей
	while (*pString == '0' && *(pString + 1))
		pString++;
	
	
	if (this->_sign)
	{
		pString--;
		*pString = '-';
	}

	char* string = new char[strlen(pString) + 1]();
	strcpy(string, pString);
	delete[] strBuffer;

	return string;
}

bool bigInt::getFromTextFile(const char* filename)
{
	FILE* pfSource = fopen(filename, "r");
	if (!pfSource)
		return false;

	fseek(pfSource, 0, SEEK_END);
	int fileSize = ftell(pfSource);
	fseek(pfSource, 0, SEEK_SET);

	char* string = new char[fileSize + 1]();
	int len = fscanf(pfSource, "%s", string);
	fclose(pfSource);

	bigInt res(string);
	*this = res;
	delete[] string;
	return true;
}

bool bigInt::saveToTextFile(const char* filename)
{
	FILE* pfDestination = fopen(filename, "w");
	if (!pfDestination)
		return false;

	char* string = this->getString();
	int len = strlen(string);
	int res = fwrite(string, sizeof(char), len, pfDestination);
	fclose(pfDestination);
	delete[] string;
	if (res < len)
		return false;
	return true;
}

bool bigInt::saveToBinFile(const char* filename)
{// в бинарный файл первым делом записывается знак. А как иначе определить, что число получилось отрицательным?
	FILE* pfDestination = fopen(filename, "w+b");
	if (!pfDestination)
		return false;

	int res = fwrite(&_sign, sizeof(_sign), 1, pfDestination);
	res = fwrite(_digits, sizeof(digit), _size, pfDestination);
	fclose(pfDestination);
	if (res < _size)
		return false;
	return true;
}

bool bigInt::getFromBinFile(const char* filename)
{// первым делом из бинарного файла считывается знак
	FILE* pfSource = fopen(filename, "r+b");
	if (!pfSource)
		return false;

	fseek(pfSource, 0, SEEK_END);
	int fileSize = ftell(pfSource);
	fseek(pfSource, 0, SEEK_SET);

	if (fileSize < sizeof(_sign))
		return false;
	int len = fread(&_sign, sizeof(_sign), 1, pfSource);
	fileSize -= sizeof(_sign);
	if (_size) delete[] _digits;
	_size = fileSize / sizeof(digit);
	_digits = new digit[_size]();
	len = fread(_digits, sizeof(digit), _size, pfSource);
	fclose(pfSource);

	return true;
}


bigInt& bigInt::operator=(const bigInt& rhv)
{
	if (this->_digits == rhv._digits)
		return *this;
	if (_size)
		delete[] _digits;
	_copy(rhv);
	return *this;
}

bigInt& bigInt::operator=(const longDigit value)
{
	if (_size)
		delete[] _digits;
	_size = 0;
	*this = bigInt(value);
	return *this;
}


bigInt bigInt::operator+(const bigInt& right) const
{
	bigInt tmp = _simpleSum(*this, right);
	tmp._delLeadZeros();
	return tmp;
}

bigInt bigInt::operator-() const
{// унарный минус
	bigInt res(*this);
	res._sign = !res._sign;
	return res;
}

bigInt bigInt::operator-(const bigInt& right) const
{
	return bigInt(*this + (-right));
}

bigInt bigInt::operator*(const bigInt& right) const
{
	return bigInt(_simpleMul(*this, right));
}

bigInt bigInt::operator/(const bigInt& right) const
{
	bigInt rem;
	return _divividing(*this, right, rem);
}

bigInt bigInt::operator%(const bigInt& right) const
{
	bigInt rem;
	_divividing(*this, right, rem);
	return rem;
}

bigInt bigInt::operator^(const bigInt& right) const
{// возведение *this в степень right
	// да, этот оператор обычно имеет другой смысл
	bigInt res = 1;
	bigInt base = *this;
	for (bigInt i = right; i > (long long int) 0; i--)
		res *= base;
	return res;
}


bigInt bigInt::operator+=(const bigInt& right)
{
	*this = bigInt(*this + right);
	return *this;
}

bigInt bigInt::operator-=(const bigInt& right)
{
	*this = bigInt(*this - right);
	return *this;
}

bigInt bigInt::operator*=(const bigInt& right)
{
	*this = bigInt(*this * right);
	return *this;
}

bigInt bigInt::operator/=(const bigInt& right)
{
	*this = bigInt(*this / right);
	return *this;
}

bigInt bigInt::operator%=(const bigInt& right)
{
	*this = bigInt(*this % right);
	return *this;
}


bigInt bigInt::operator++()
{
	bigInt oldVal = *this;
	*this = *this + 1;
	return oldVal;
}

bigInt bigInt::operator++(int)
{
	*this = *this + 1;
	return *this;
}

bigInt bigInt::operator--()
{
	bigInt oldVal = *this;
	*this = *this - 1;
	return oldVal;
}

bigInt bigInt::operator--(int)
{
	*this = *this - 1;
	return *this;
}


bool bigInt::operator>(const bigInt& B)
{
	return this->_cmp(B) > 0;
}

bool bigInt::operator>=(const bigInt& B)
{
	return this->_cmp(B) >= 0;
}

bool bigInt::operator<(const bigInt& B)
{
	return this->_cmp(B) < 0;
}

bool bigInt::operator<=(const bigInt& B)
{
	return this->_cmp(B) <= 0;
}

bool bigInt::operator==(const bigInt& B)
{
	return this->_cmp(B) == 0;
}

bool bigInt::operator!=(const bigInt& B)
{
	return this->_cmp(B) != 0;
}


std::ostream& operator<<(std::ostream &out, bigInt A)
{
	char* str = A.getString();
	out << str;
	delete[] str;
	return out;
}

std::istream& operator>>(std::istream &is, bigInt &A)
{
	char string[1000];
	is >> string;
	bigInt res(string);
	A = res;
	return is;
}



char* bigInt::_viewNumber()
{// функция для отладки. Возвращает содержимое числа, но не в 10-ричном виде, а в том виде, в котором оно хранится
	char* string = new char[10000]();
	if (_sign)
		string[0] = '-';

	for (int i = _size - 1; i + 1; i--)
	{
		char tmp[100];
		sprintf(tmp, "%u ", _digits[i]);
		strcat(string, tmp);
		/*	digit t = _digits[i];
		printf("%d ", t);*/
	}
	return string;
}

void bigInt::_setSize(int size)
{	// изменяет размер числа, при этом обнуляя его. Необходимо, например, для произведения,
	// когда, в общем случае, размерность результата равна сумме размерностей сомножителей
	if (_size)
		delete[] _digits;
	_size = size;
	_sign = 0;
	_digits = new digit[_size]();
}

digit & bigInt::operator[](int i)
{
	if (i < 0)
		return _digits[_size + i];
	return this->_digits[i];
}

digit bigInt::operator[](int i) const
{
	if (i < 0)
		return _digits[_size + i];
	return this->_digits[i];
}

void bigInt::_copy(const bigInt &rhv)
{
	_size = rhv._size;
	if (!_size)
	{
		_digits = NULL;
		_sign = rhv._sign;
		return;
	}
	_digits = new digit[_size];
	_sign = rhv._sign;
	memcpy(_digits, rhv._digits, _size*sizeof(digit));
	return;
}

void bigInt::_delLeadZeros()
{
	while ((_size - 1) && _digits && _digits[_size - 1] == 0)
		_size--;
	if (_size == 1 && _digits[0] == 0)
		_sign = 0;
}

longDigit bigInt::_cmp(const bigInt& B)
{
	// функция возвращает
	// 0 - если числа равны,
	// >0 - если this больше
	// <0 - если this меньше
	int thisSign;
	if (this->_sign == 0)
		thisSign = 1;
	else
		thisSign = -1;

	if (this->_sign == B._sign)
	{// если числа одного знака
		if (this->_size > B._size)
		{
			return thisSign;
		}
		else if (this->_size < B._size)
		{
			return -thisSign;
		}
		else
		{// если длины чисел равны
			int i = this->_size - 1;
			while ((i + 1) && this->_digits[i] == B[i])
				i--;

			if (i == -1)
			{// в случае, если числа равны
				return 0;
			}
			return ((longDigit) this->_digits[i] - (longDigit)B._digits[i])*thisSign;
		}
	}
	else
		return thisSign;
	return 0;
}

void bigInt::_shiftLeft(int s)
{// сдвигает число на s разрядов вправо
	// то есть, по сути, это умножение на BASE^s
	// сдвиг на отрицательное s - деление на BASE^(-s)
	digit* newDig = new digit[_size + s]();
	for (int i = 0; i < _size; i++)
	{
		if (i + s >= 0)
		{
			newDig[i + s] = _digits[i];
		}
	}
	delete[] _digits;
	_digits = newDig;
	_size += s;
	_delLeadZeros();
}

const bigInt _simpleSum(const bigInt& left, const bigInt& right)
{
	// выглядит очень длинным и страшным. Но на самом деле оно простое, 
	// много места занимают ассемблерные вставки для Visual Studio и GCC, закомментиованные участки кода, 
	// которые используются, когда BASE != 2^32
	bigInt A = left, B = right; // в А будет большее по модулю число, в B - меньшее.
	A._sign = 0;
	B._sign = 0;
	if (A > B)
	{
		A._sign = left._sign;
		B._sign = right._sign;
	}
	else
	{
		A = right;
		B = left;
	}

	if (A._sign == B._sign)
	{// если числа одного знака, то просто складываем их и выставляем нужный знак
		A._size++;
		bigInt res = A; // в res сначала записываем копию A, но чуть большей размерности.
						// Это сделано для небольшой оптимизации в дальнейшем, для случаев,
						// когда размерность B значительно меньше размерности A
						// и есть только небольшое количество переносов из младших разрядов в старшие
		A._size--;
		digit carryAsm = 0;

		// прибавляем число меньшей размерности к числу большей размерности
		for (int i = 0; i < B._size; i++)
		{
			// вот что, по сути, делают ассемблерные вставки
			/*longDigit tmp = A[i];
			tmp += B[i];
			tmp += carry;
			digit norm;
			carry = res._normalize(tmp, norm);
			res[i] = norm;*/

			digit Ai = A[i];
			digit Bi = B[i];
			digit normAsm = 0;

			// ассемблерная вставка для VisualStudio (согласна работать только с __asm {})
#ifdef forVS
			__asm
			{
				xor	ecx, ecx;
				mov	eax, Ai;
				mov ebx, Bi;

				add	eax, ebx;
				jnc plusCarry;
				inc ecx;

			plusCarry:
				mov	ebx, carryAsm;
				add	eax, ebx;
				jnc	exitAsmSum1;
				inc	ecx;
			exitAsmSum1:
				mov	carryAsm, ecx;
				mov	normAsm, eax;
			}
#endif
			// ассемблерная вставка для GCC
#ifdef forGCC
			asm volatile   (
				"xorl	%%ecx, %%ecx;"
				"addl	%%ebx, %%eax;"
				"jnc	addCarry;"
				"incl	%%ecx;"
				"addCarry:	addl	%%edx, %%eax;"
				"jnc	exitSum1;"
				"incl	%%ecx;"
				"exitSum1:;" : "=a" (normAsm), "=c" (carryAsm) : "a" (Ai), "b" (Bi), "d" (carryAsm));
#endif

			res[i] = normAsm;
		}
		// добавляем необходимое количество переносов
		// условие (carry != 0) для выхода из цикла и есть та самая небольшая оптимизация
		for (int i = B._size; (i < A._size) && carryAsm; i++)
		{
			/*longDigit tmp = A[i];
			tmp += carry;
			digit norm;
			carry = res._normalize(tmp, norm);
			res[i] = norm;*/

			digit Ai = A[i];
			digit normAsm = 0;
			// ассемблерная вставка для VisualStudio
#ifdef forVS
			__asm
			{
				xor	ecx, ecx;
				mov	eax, Ai;

				mov	ebx, carryAsm;
				add	eax, ebx;
				jnc	exitAsmSum2;
				inc	ecx;
			exitAsmSum2:
				mov	carryAsm, ecx;
				mov	normAsm, eax;
			}
#endif

			// ассемблерная вставка для GCC
#ifdef forGCC
			asm volatile (
				"xorl	%%ecx, %%ecx;"
				"addl	%%edx, %%eax;"
				"jnc	exitSum2;"
				"incl	%%ecx;"
				"exitSum2:;" : "=a" (normAsm), "=c" (carryAsm) : "a" (Ai), "d" (carryAsm));
#endif
			res[i] = normAsm;
		}
		res[A._size] = carryAsm;
		res._sign = A._sign;
		return res;
	}
	else
	{// отнимаем одно от другого и выставляем нужный знак
		bigInt res = A;
		digit carryAsm = 0;
		for (int i = 0; i < B._size; i++)
		{
			/*longDigit tmp = A[i];
			tmp -= B[i];
			tmp -= carry;
			digit norm;
			carry = res._normalize(tmp, norm);
			res[i] = norm;*/

			digit Ai = A[i];
			digit Bi = B[i];
			digit normAsm = 0;

			// ассемблерная вставка для Visual studio
#ifdef forVS
			__asm
			{
				xor	ecx, ecx;
				mov	eax, Ai;
				mov ebx, Bi;

				sub	eax, ebx;
				jnc minusCarry;
				inc ecx;

			minusCarry:
				mov	ebx, carryAsm;
				sub	eax, ebx;
				jnc	exitAsmSub1;
				inc	ecx;
			exitAsmSub1:
				mov	carryAsm, ecx;
				mov	normAsm, eax;
			}
#endif

			// ассемблерная вставка для GCC
#ifdef forGCC
			asm volatile (
				"xorl	%%ecx, %%ecx;"
				"subl	%%ebx, %%eax;"
				"jnc	subCarry;"
				"incl	%%ecx;"
				"subCarry:	subl	%%edx, %%eax;"
				"jnc	exitSum3;"
				"incl	%%ecx;"
				"exitSum3:;" : "=a" (normAsm), "=c" (carryAsm) : "a" (Ai), "b" (Bi), "d" (carryAsm)); 
#endif
			res[i] = normAsm;
		}

		for (int i = B._size; (i < A._size) && carryAsm; i++)
		{
			/*longDigit tmp = A[i];
			tmp -= carry;
			digit norm;
			carry = res._normalize(tmp, norm);
			res[i] = norm;*/

			digit Ai = A[i];
			digit normAsm = 0;

			// ассемблерная вставка для Visual studio
#ifdef forVS
			__asm
			{
				xor	ecx, ecx;
				mov	eax, Ai;
				mov	ebx, carryAsm;
				sub	eax, ebx;
				jnc	exitAsmSub2;
				inc	ecx;
			exitAsmSub2:
				mov	carryAsm, ecx;
				mov	normAsm, eax;
			}
#endif

			// ассемблерная вставка для GCC
#ifdef forGCC
			asm volatile (
				"xorl	%%ecx, %%ecx;"
				"subl	%%edx, %%eax;"
				"jnc	exitSum4;"
				"incl	%%ecx;"
				"exitSum4:;" : "=a" (normAsm), "=c" (carryAsm) : "a" (Ai), "d" (carryAsm));
#endif
			res[i] = normAsm;
		}
		res._sign = A._sign;
		return res;
	}
	bigInt res;
	return res;
}

const bigInt _simpleMul(const bigInt& A, const bigInt& B)
{// простое умножение "столбиком"
	// выглядит большим и страшным из-за ассемблерных вставок и закомментированного кода
	bigInt res;
	res._setSize(A._size + B._size);
	//digit carry = 0;
	digit carryAsm = 0;
	for (int i = 0; i < A._size; i++)
	{// умножение младшего разряда B на число A
		
		/*unsLongDigit tmp = A[i];
		tmp *= B[0];
		tmp += carry;
		digit norm;
		carry = tmp / BASE;
		norm = tmp % BASE;
		res[i] = norm;*/

		digit Ai = A[i];
		digit B0 = B[0];
		digit normAsm = 0;

		// ассемблерная вставка для VisualStudio
#ifdef forVS
		__asm
		{
			mov	eax, Ai;
			mov	ebx, B0;
			mul	ebx; // результат в edx:eax
			mov	ebx, carryAsm;
			add	eax, ebx;
			jnc	exitMul1;
			inc	edx;
		exitMul1:
			mov	normAsm, eax;
			mov carryAsm, edx;
		}
#endif

		// ассемблерная вставка для GCC
#ifdef forGCC
		asm volatile (
			"mull	%%ebx;"
			"add	%%ecx, %%eax;"
			"jnc	exitMul1;"
			"incl	%%edx;"
			"exitMul1:;" : "=a" (normAsm), "=d" (carryAsm) : "a" (Ai), "b" (B0), "c" (carryAsm));
#endif
		res[i] = normAsm;

	}
	res[A._size] = carryAsm;

	for (int i = 1; i < B._size; i++)
	{
		carryAsm = 0;
		for (int j = 0; j < A._size; j++)
		{
			/*unsLongDigit tmp = B[i];
			tmp *= A[j];
			tmp += res[i + j];
			tmp += carry;
			digit norm;
			carry = tmp / BASE;
			norm = tmp % BASE;*/

			digit Aj = A[j];
			digit Bi = B[i];
			digit Ri = res[i + j];
			digit normAsm = 0;

			// ассемблерная вставка для VisualStudio
#ifdef forVS
			__asm
			{
				mov	eax, Aj;
				mov	ebx, Bi;
				mul	ebx; // результат в edx:eax
				mov	ebx, Ri;
				add	eax, ebx;
				jnc	addCarry;
				inc	edx;
			addCarry:
				mov	ebx, carryAsm;
				add	eax, ebx;
				jnc	exitMul2;
				inc	edx;
			exitMul2:
				mov	normAsm, eax;
				mov carryAsm, edx;
			}
#endif

			// ассемблерная вставка для GCC
#ifdef forGCC
			asm volatile (
				"mull	%%ebx;"
				"add	%%edi, %%eax;"
				"jnc	addCarryMul2;"
				"incl	%%edx;"
				"addCarryMul2:;"
				"add	%%ecx, %%eax;"
				"jnc	exitMul2;"
				"incl	%%edx;"
				"exitMul2:;" : "=a" (normAsm), "=d" (carryAsm) : "a" (Aj), "b" (Bi), "c" (carryAsm), "D"(Ri));
#endif
			res[i + j] = normAsm;
		}
		res[i + A._size] = carryAsm;
	}

	res._sign = (!A._sign && B._sign) || (A._sign && !B._sign);
	res._delLeadZeros();
	return res;
}

const bigInt _divividing(const bigInt& A, const bigInt& B, bigInt &remainder)
{// возвращает целую часть от деления, в remainder - остаток
	remainder = A;
	remainder._sign = 0;

	bigInt divider = B;
	divider._sign = 0;

	if (divider == bigInt((long long int) 0))
	{
		throw DIVISION_BY_ZERO;
		return bigInt(-1);
	}

	if (remainder < divider)
	{
		remainder = A;
		return bigInt((long long int) 0);
	}

	if (remainder._size == divider._size && (remainder._size == 1))
	{
		bigInt res;
		res._setSize(1);
		res[0] = remainder[0] / divider[0];
		remainder[0] = remainder[0] % divider[0];
		res._sign = (!A._sign && B._sign) || (A._sign && !B._sign);
		return res;
	}

	if (divider._size == 1)
	{
		remainder._setSize(1);
		bigInt res;
		res._setSize(A._size);
		digit carry = 0;
		for (int i = A._size - 1; i >= 0; i--)
		{
			long long int temp = carry;
			temp *= BASE;
			temp += A[i];
			res[i] = temp / divider[0];
			carry = temp - res[i] * divider[0];
		}
		remainder._sign = (!A._sign && B._sign) || (A._sign && !B._sign);
		remainder[0] = carry;
		remainder._delLeadZeros();
		res._sign = (!A._sign && B._sign) || (A._sign && !B._sign);
		res._delLeadZeros();
		return res;
	}
	return _divColumn(A, B, remainder);
	return _divBinSearch(A, B, remainder); // 2 варианта деления, но _divColumn должен работать быстрее
}

const bigInt _divColumn(const bigInt& A, const bigInt& B, bigInt &remainder)
{
	remainder = A;
	remainder._sign = 0;

	bigInt divider = B;
	divider._sign = 0;

	// деление большого числа на большое число "столбиком"
	bigInt res;
	res._setSize(A._size - B._size + 1);

	unsLongDigit scale = BASE;
	scale += 1; // "Масштабирующий множитель" для того, чтобы divider[-1] >= BASE/2
	scale /= (2 * (divider[-1]));
	if (scale > 1)
	{
		remainder *= scale;
		divider *= scale;
	}

	// вставка ведущего 0 в remainder
	bigInt tmp;
	tmp._setSize(remainder._size + 1);
	for (int i = 0; i < remainder._size; i++)
		tmp[i] = remainder[i];
	remainder = tmp;

	for (int vi = A._size - B._size, uj = A._size; vi >= 0; vi--, uj--)
	{
		// сделано пошагово для избежания ошибок переполнения
		unsLongDigit qGuess = remainder[uj];
		qGuess *= BASE;
		qGuess += remainder[uj - 1];

		unsLongDigit r = qGuess % divider[-1];
		qGuess /= divider[-1];

		while (r < BASE)
		{// при больших BASE этот цикл выполняется довольно долго
			unsLongDigit temp2 = divider[-2] * qGuess;
			unsLongDigit temp1 = r*BASE;
			temp1 += remainder[-2];
			if ((temp2 > temp1) || (qGuess == BASE))
			{
				--qGuess;
				r += divider[-1];
			}
			else break;
		}
		bigInt qMulDivider = qGuess;
		qMulDivider *= divider;

		// отнять от remainder qMulDivider, точнее только от необходимых разрядов
		bigInt remTmp = remainder;
		remTmp._shiftLeft(-vi);
		remTmp._delLeadZeros();
		while (qMulDivider > remTmp)
		{
			qGuess--;
			qMulDivider = qGuess;
			qMulDivider *= divider;
		}
		remTmp = _simpleSum(remTmp, -qMulDivider);
		for (int i = 0; i < remTmp._size; i++)
			remainder[vi + i] = remTmp[i];
		bigInt q = qGuess;
		q._shiftLeft(vi);
		res += q;
	}

	remainder._delLeadZeros();
	if (scale > 1)
	{
		remainder /= scale;
		divider /= scale;
	}
	while (remainder >= divider)
	{// почему-то при делении нацело возникают моменты, что остаток оказался больше делителя
		remainder -= divider;
		res++;
	}

	res._sign = (!A._sign && B._sign) || (A._sign && !B._sign);
	remainder._sign = (!A._sign && B._sign) || (A._sign && !B._sign);
	res._delLeadZeros();
	return res;
}

const bigInt _divBinSearch(const bigInt& A, const bigInt& B, bigInt &remainder)
{	// деление "столбиком"
	// подбор очередного разряда частного бинарным поиском
	remainder = A;
	remainder._sign = 0;

	bigInt divider = B;
	divider._sign = 0;

	bigInt res;
	res._setSize(A._size - B._size + 1);

	for (int i = A._size - B._size + 1; i; i--)
	{
		longDigit qGuessMax = BASE; // для того, чтобы qGuessMin могло быть равно BASE - 1
		longDigit qGuessMin = 0;
		longDigit qGuess = qGuessMax;

		// цикл - подбор бинарным поиском числа qGuess
		while (qGuessMax - qGuessMin > 1)
		{
			qGuess = (qGuessMax + qGuessMin) / 2;

			// получаем tmp = qGuess * divider * BASE^i;
			bigInt tmp = divider * qGuess;
			tmp._shiftLeft(i - 1);
			bigInt tmp2 = remainder;
			tmp2._delLeadZeros();

			if (tmp > tmp2)
				qGuessMax = qGuess;
			else
				qGuessMin = qGuess;
		}
		// remainder -= divider * qGuessMin, точнее вычитание только из необходимых разрядов
		bigInt qMulDivider = divider * qGuessMin;
		bigInt remTmp = remainder;
		remTmp._shiftLeft(-i + 1);
		remTmp._delLeadZeros();

		remTmp = _simpleSum(remTmp, -qMulDivider); // нельзя использовать "-", так как в нём удаляются ведущие 0.

		// записываем изменившиеся разряды в remainder
		for (int j = 0; j < remTmp._size; j++)
			remainder[i - 1 + j] = remTmp[j];

		res[i - 1] = qGuessMin;
	}

	res._sign = (!A._sign && B._sign) || (A._sign && !B._sign);
	remainder._sign = (!A._sign && B._sign) || (A._sign && !B._sign);
	remainder._delLeadZeros();
	res._delLeadZeros();

	return res;
}

const bigInt pow(const bigInt& A, const bigInt& B, bigInt& modulus)
{// возведение A в степень B по модулю modulus
	if (modulus == (longDigit)0)
		return A ^ B;

	bigInt base = A % modulus;
	bigInt res = 1;
	for (bigInt i = B; i > (long long int) 0; i--)
		res = (res * base) % modulus;
	return res;
	
}
