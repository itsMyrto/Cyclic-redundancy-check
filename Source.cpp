#include <iostream>
#include <cstring>
#include <random>
#include <chrono>
#define MAX 1000000
using namespace std;
using namespace chrono_literals;
using chrono::system_clock;

bool equalMessages(int* D, int* received, int k) {
	for (int i = 0; i < k; i++) {
		if (received[i] != D[i]) {
			return false;
		}
	}
	return true;
}

int exor(int a, int b) {
	if (a == b) {
		return 0;
	}
	else {
		return 1;
	}
}

int* modulo2Division(int* D,int* P,int k,int n) {
	
	int* R = new int[k-1];
	int* temp = new int[k];

	int zeroBits = 0;
	for (int i = 0; i < n; i++) {
		if (D[i] == 0) {
			zeroBits++;
		}
		else {
			break;
		}
	}

	if (zeroBits == n) {
		for (int i = 0; i < k - 1; i++) {
			R[i] = 0;
		}

		return R;
	}

	int t_length = n - zeroBits;
	int* T = new int[t_length];
	for (int i = 0; i < t_length; i++) {
		T[i] = D[zeroBits + i];
	}

	for (int i = 0; i < k; i++) {
		temp[i] = T[i];
	}

	int posT = k;

	while (true) {
		
		for (int i = 0; i < k - 1; i++) {
			R[i] = exor(temp[i+1], P[i+1]);
		}


		int leadingZeros = 0;
		for (int i = 0; i < k - 1; i++) {
			if (R[i] == 0) {
				leadingZeros++;
			}
			else {
				break;
			}
		}


		if (leadingZeros >= t_length - posT) {

			int shift = t_length - posT;
			int position_of_1 = 0;
			for (int i = 0; i < k - 1; i++) {
				if (R[i] == 1) {
					position_of_1 = i;
					break;
				}
			}

			int pos;
			for (int i = position_of_1; i < k - 1; i++) {
				R[i - shift] = R[i];
				pos = i;
			}

			pos = 0;
			for (int i = posT; i < t_length; i++) {
				R[k-2-pos] = T[i];
				pos++;
			}

			break;
		}
		else {

			int position_of_1 = -1;
			for (int i = 0; i < k - 1; i++) {
				if (R[i] == 1) {
					position_of_1 = i;
					break;
				}
			}

			if (position_of_1 == -1) {
				while (posT < t_length) {
					if (T[posT] == 1) {
						break;
					}

					posT++;
				}
				
				if (posT == t_length) {
					break;
				}

				for (int i = 0; i < k; i++) {
					temp[i] = T[posT];
					posT++;
				}

				continue;
			}

			int pos = 0;
			for (int i = position_of_1; i < k - 1; i++) {
				temp[pos] = R[i];	
				pos++;
			}
			for (int i = pos; i < k; i++) {
				temp[pos] = T[posT];
				pos++;
				posT++;	
			}

		}
		
	}
	
	delete[] T;
	delete[] temp;

	return R;

}

int main() {

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0.0, 1.0);

	string number;
	int k;
	double BER;
	cout << "Enter k: " << endl;
	cin >> k;
	cout << "Enter the number P: " << endl;
	cin >> number;
	cout << "Enter the Bit Error Rate: " << endl;
	cin >> BER;
	
	int p_length = number.length();
	int n = p_length + k - 1;
	int messages_with_errors = 0;
	int errors_CRC_detects = 0;
	int errors_CRC_does_not_detect = 0;
	int samples = 0;

	int* P = new int[p_length];
	for (int i = 0; i < p_length; i++) {
		if (number[i] == '1') {
			P[i] = 1;
		}
		else {
			P[i] = 0;
		}
	}

	while (samples < MAX) {

		//This is the message that source wants to send
		int* D = new int[k];
				
		D[0] = 1;
		for (int i = 1; i < k; i++) {
			double randomBitPossibility = dis(gen);
			if (randomBitPossibility < 0.5) {
				D[i] = 0;
			}
			else {
				D[i] = 1;
			}
		}

		//Shifting D -> Adding n-k zero's in the end 
		int* D_new = new int[n];
		for (int i = 0; i < k; i++) {
			D_new[i] = D[i];
		}
		for (int i = k; i < n; i++) {
			D_new[i] = 0;
		}

		//Calculating F using modulo 2
		int* F = modulo2Division(D_new, P, p_length, n);

		//T=DF
		int* T = new int[n];
		for (int i = 0; i < k; i++) {
			T[i] = D[i];
		}
		int pos = 0;
		for (int i = k; i < n; i++) {
			T[i] = F[pos];
			pos++;
		}

		//Recieving the message (the bits might be changed)
		int* receivedMessage = new int[n];
		for (int i = 0; i < n; i++) {
			double random = dis(gen);
			receivedMessage[i] = T[i];
			if (BER > random) {
				if (T[i] == 1) {
					receivedMessage[i] = 0;
				}
				else {
					receivedMessage[i] = 1;
				}
			}
		}
		
		//Calculating the remainder
		int* R = modulo2Division(receivedMessage, P, p_length, n);

		//Checking if T modulo2 P gives 0
		bool messageIsAccepted = true;
		for (int i = 0; i < p_length - 1; i++) {
			if (R[i] != 0) {
				messageIsAccepted = false;
				break;
			}
		}
		
		if (messageIsAccepted == false) {
			errors_CRC_detects++;
		}

		//Checking if the received message is different from the message that was actually sent 
		if (equalMessages(T, receivedMessage, n) == false) {
			messages_with_errors++;
			if (messageIsAccepted == true) {
				errors_CRC_does_not_detect++;
			}
		}

		samples++;

		
		
		delete[] D;
		delete[] D_new;
		delete[] T;
		
	}


	cout << "From 1.000.000 samples: " << endl;
	cout << "->The number of messages with error: " << messages_with_errors << endl;
	cout << "->The number of messages that CRC disapproved: " << errors_CRC_detects << endl;
	cout << "->The number of messages that CRC approved BUT they had errors: " << errors_CRC_does_not_detect << endl;
	cout << "->The rate of the messages with error = (Messages with errors/Samples) * 100 is " << (messages_with_errors / (MAX*1.0)) * 100 << "%" << endl;
	cout << "->The rate of the messages that were detected by the CRC = (Messages that CRC disapproved/Samples) * 100 is " << (errors_CRC_detects / (MAX*1.0)) * 100 << "%" << endl;
	cout << "->The rate of the messages that CRC didn't detect but they had error = (Messages that CRC incorrectly approved/Samples) * 100 is " << (errors_CRC_does_not_detect / (MAX*1.0)) * 100 << "%" << endl;
	

	return 0;

}

/*
cout << "R test: " << endl;
for (int i = 0; i < k - 1; i++) {
	cout << R[i] << " ";
}
cout << endl;


cout <<"What is going on: " << t_length <<" " <<posT << endl;
cout << "Leading Zeros: " << leadingZeros << endl;
*/

/*
cout << "R is: " << endl;
for (int i = 0; i < k - 1; i++) {
	cout << R[i] << " ";
}
cout << endl;
cout << endl;
*/

/*
cout << "D: ";
for (int i = 0; i < k; i++) {
	cout << D[i];
}
cout << "   T: ";
for (int i = 0; i < n; i++) {
	cout << T[i];
}
cout << "   received: ";
for (int i = 0; i < n; i++) {
	cout << receivedMessage[i];
}
cout << "   R is: ";
for (int i = 0; i < p_length-1; i++) {
	cout << R[i];
}
*/