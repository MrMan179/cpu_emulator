//    Von Neumann Architecture CPU Emulator written in C++
//    Copyright (C) 2026  Reuben Stannard
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <iostream>
#include <cstring>
#include <bitset>

struct bus;
class reg8;
class alu;
class cpu;

struct bus
{
	std::bitset<8> data;
};

class reg8
{
	public:
		int RUN(cpu &CPU);

		std::bitset<8> data;
		std::bitset<4> code = (char)0b1100;

		int update_value(bus &Bus)
		{
			data = Bus.data;
			return 0;
		}
		
		int get_value(bus &Bus)
		{
			Bus.data = data;
			return 0;
		}

		int set_code(std::bitset<4> new_code)
		{
			code = new_code;
			return 0;
		}
};

class alu
{
	public:
		int RUN(cpu &CPU);
		reg8 BUF;
	private:
		int ADD(std::bitset<8> a, std::bitset<8> b, std::bitset<1> ca, void* co, std::bitset<1> op_select, bus &out_bus)
		{
			std::bitset<8> out = (char)0;
			std::bitset<1> carry = op_select[0] ? std::bitset<1>(1) : ca;
			auto* carry_output = static_cast<std::bitset<1>*>(co);
			
			for (int i = 0; i < 8; i++)
			{
				bool bit_a = a[i];
				bool bit_b = b[i] ^ op_select[0];
				bool c = carry[0];

				bool sum = bit_a ^ bit_b ^ c;
				if (sum)
				{
					out.set(i);
				} else
				{
					out.reset(i);
				}

				if ((bit_a & bit_b) | (c & (bit_a ^ bit_b)))
				{
					carry.set(0);
				} else
				{
					carry.reset(0);
				}
			}
			
			out_bus.data = out;
			*carry_output = carry;
			return 0;
		}

		int AND(std::bitset<8> a, std::bitset<8> b, bus &out_bus)
		{
			std::bitset<8> out = (char)0;
			for (int i = 0; i < 8; i++)
			{
				if (a[i] & b[i]) out.set(i);
				else out.reset(i);
			}
			out_bus.data = out;
			return 0;
		}


		int OR(std::bitset<8> a, std::bitset<8> b, bus &out_bus)
		{
			std::bitset<8> out = (char)0;
			for (int i = 0; i < 8; i++)
			{
				if (a[i] | b[i]) out.set(i);
				else out.reset(i);
			}
			out_bus.data = out;
			return 0;
		}

		int XOR(std::bitset<8> a, std::bitset<8> b, bus &out_bus)
		{
			std::bitset<8> out = (char)0;
			for (int i = 0; i < 8; i++)
			{
				if (a[i] ^ b[i]) out.set(i);
				else out.reset(i);
			}
			out_bus.data = out;
			return 0;
		}

		int NOT(std::bitset<8> a, bus &out_bus)
		{
			std::bitset<8> out = (char)0;
			for (int i = 0; i < 8; i++)
			{
				if (!a[i]) out.set(i);
				else out.reset(i);
			}
			out_bus.data = out;
			return 0;
		}

		int SHL(std::bitset<8> a, void* co, bus &out_bus)
		{
			std::bitset<8> out = (char)0;
			auto* carry_out = static_cast<std::bitset<1>*>(co);
			for (int i = 0; i < 7; i++)
			{
				if (a[i]) out.set(i+1);
				else out.reset(i+1);
			}
			std::bitset<1> carry = (char)0;
			if (a[7]) carry.set(0);
			else carry.reset(0);
			*carry_out = carry;
			out_bus.data = out;
			return 0;
		}

		int SHR(std::bitset<8> a, void* co, bus &out_bus)
		{
			std::bitset<8> out = (char)0;
			auto* carry_out = static_cast<std::bitset<1>*>(co);
			for (int i = 1; i < 8; i++)
			{
				if (a[i]) out.set(i-1);
				else out.reset(i-1);
			}
			std::bitset<1> carry = (char)0;
			if (a[0]) carry.set(0);
			else carry.reset(0);
			*carry_out = carry;
			out_bus.data = out;
			return 0;
		}
};

class cpu
{
	public:
		reg8 MAR {0};
		reg8 MDR {0};
		reg8 PC {0}; 
		reg8 ACC {0};
		reg8 STAT {0};
	
		reg8 AX {0}; 
		reg8 BX {0}; 
		reg8 CX {0}; 
		reg8 DX {0}; 
		
		alu ALU;
	
		bus internal_data_bus;
		bus internal_addr_bus;
		bus internal_ctrl_bus;
	
		bus data_bus;
		bus addr_bus;
		bus ctrl_bus;

		int INIT()
		{
			MAR.set_code(0b1000);
			MDR.set_code(0b1001);
			PC.set_code(0b1010);
			ACC.set_code(0b1011);
			STAT.set_code(0b0010);

			AX.set_code(0b1100);
			BX.set_code(0b1101);
			CX.set_code(0b1110);
			DX.set_code(0b1111);
			
			return 0;
		}

		int RUN()
		{
			MAR.RUN(*this);
			MDR.RUN(*this);
			PC.RUN(*this);
			ACC.RUN(*this);
			STAT.RUN(*this);
			AX.RUN(*this);
			BX.RUN(*this);
			CX.RUN(*this);
			DX.RUN(*this);

			ALU.RUN(*this);

			return 0;
		}
};

int reg8::RUN(cpu &CPU)
{
	std::bitset<8> ctrl = CPU.internal_ctrl_bus.data;
	if (ctrl[0] == code[0] and ctrl[1] == this->code[1] and ctrl[2] == this->code[2] and ctrl[3] == this->code[3])
	{
		if (not ctrl[4]) this->update_value(CPU.internal_data_bus);
		else this->get_value(CPU.internal_data_bus);
	}
	return 0;
}

int alu::RUN(cpu &CPU)
{
	std::bitset<8> ctrl = CPU.internal_ctrl_bus.data;
	if (ctrl[0] and not ctrl[1] and not ctrl[2] and not ctrl[3])
	{
		std::bitset<1> carry = (std::bitset<1>)0;

		if (not ctrl[4])
		{
			if (not ctrl[5] and not ctrl[6] and not ctrl[7]) this->ADD(CPU.ACC.data, this->BUF.data, 0, &carry, 0, CPU.internal_data_bus);
			else if (not ctrl[5] and not ctrl[6] and ctrl[7]) this->ADD(CPU.ACC.data, this->BUF.data, 0, &carry, 1, CPU.internal_data_bus);
			else if (not ctrl[5] and ctrl[6] and not ctrl[7]) this->AND(CPU.ACC.data, this->BUF.data, CPU.internal_data_bus);
			else if (not ctrl[5] and ctrl[6] and ctrl[7]) this->OR(CPU.ACC.data, this->BUF.data, CPU.internal_data_bus);
			else if (ctrl[5] and not ctrl[6] and not ctrl[7]) this->XOR(CPU.ACC.data, this->BUF.data, CPU.internal_data_bus);
			else if (ctrl[5] and not ctrl[6] and ctrl[7]) this->NOT(CPU.ACC.data, CPU.internal_data_bus);
			else if (ctrl[5] and ctrl[6] and not ctrl[7]) this->SHL(CPU.ACC.data, &carry, CPU.internal_data_bus);
			else if (ctrl[5] and ctrl[6] and ctrl[7]) this->SHR(CPU.ACC.data, &carry, CPU.internal_data_bus);

			if (carry[0]) CPU.STAT.data.set(0);
		} else
		{
			this->BUF.update_value(CPU.internal_data_bus);
		}
	}

	return 0;
}

int main()
{
	cpu CPU;
	CPU.INIT();
	
	// Set AX to 10110011
	CPU.internal_data_bus.data = (char)0b10110011;
	CPU.internal_ctrl_bus.data = (char)0b00001100;
	CPU.RUN();

	// Set BX to 11001100
	CPU.internal_data_bus.data = (char)0b11001100;
	CPU.internal_ctrl_bus.data = (char)0b00001101;
	CPU.RUN();

	// Get AX to internal_data_bus
	CPU.internal_ctrl_bus.data = (char)0b00011100;
	CPU.RUN();

	// Set ACC to internal_data_bus
	CPU.internal_ctrl_bus.data = (char)0b00001011;
	CPU.RUN();

	// Get BX to internal_data_bus
	CPU.internal_ctrl_bus.data = (char)0b00011101;
	CPU.RUN();

	// Set ALU.BUF to internal_data_bus
	CPU.internal_ctrl_bus.data = (char)0b00010001;
	CPU.RUN();

	// Perform addition calculation
	CPU.internal_ctrl_bus.data = (char)0b00000001;
	CPU.RUN();

	// Set CX to internal_data_bus
	CPU.internal_ctrl_bus.data = (char)0b00001110;
	CPU.RUN();

	std::cout << "AX: " << CPU.AX.data << '\n';
	std::cout << "BX: " << CPU.BX.data << '\n';
	std::cout << "CX: " << CPU.CX.data << '\n';
	std::cout << "ACC: " << CPU.ACC.data << '\n';
	std::cout << "ALU.BUF: " << CPU.ALU.BUF.data << '\n';
	std::cout << "STAT: " << CPU.STAT.data << '\n';

	return 0;
}
