#pragma once
using namespace std;

template<class T>
void QuickSort( T *arr, int _start, int _end )
{
	if (_start < _end)
	{
		int standard = partition( arr, _start, _end );
		QuickSort( arr, _start, standard - 1 );			// 기준의 왼쪽 정렬
		QuickSort( arr, standard + 1, _end );			// 기준의 오른쪽 정렬
	}
}

template<class T>
int partition( T *arr, int _start, int _end )
{
	int standardIdx;
	int i = _start;
	int leftIdx = 0;
	int rightIdx = 0;
	T* leftTmp = new T[_end + 1];
	T* rightTmp = new T[_end + 1];

	while (i < _end)
	{
		if (arr[i] < arr[_end])		// 기준보다 작거나 같으면 왼쪽
		{
			leftTmp[leftIdx++] = arr[i];
		}
		else							// 기준보다 크면 오른쪽
		{
			rightTmp[rightIdx++] = arr[i];
		}
		i++;
	}

	int j = _start;

	int l = 0;
	while (l < leftIdx)
	{
		arr[j] = leftTmp[l];
		j++;
		l++;
	}

	standardIdx = j;
	arr[j++] = arr[_end];

	int k = 0;
	while (j <= _end)
	{
		arr[j++] = rightTmp[k++];
	}

	delete[ ] rightTmp;
	delete[ ] leftTmp;

	return standardIdx;
}
