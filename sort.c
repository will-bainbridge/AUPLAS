#include "auplas.h"

void sift_down(int *index, double *value, int lower, int upper)
{
        int i = lower, c = lower, lastindex = upper/2;

        int itemp;
        double vtemp;

        itemp = index[i]; vtemp = value[i];

        while(c <= lastindex)
        {
                c = 2*i;
                if((c + 1 <= upper) && (index[c + 1] > index[c])) c++;

                if(itemp >= index[c]) break;

                index[i] = index[c];
                value[i] = value[c];
                i = c;
        }

        index[i] = itemp; value[i] = vtemp;
}

void heap_sort(int *index, double *value, int n)
{
        int i;
        int itemp;
        double vtemp;

        for(i = n/2; i >= 1; i--)
                sift_down(index - 1, value - 1, i, n);

        for(i = n; i >= 2; i--)
        {
                itemp = index[0];      vtemp = value[0];
                index[0] = index[i-1]; value[0] = value[i-1];
                index[i-1] = itemp;    value[i-1] = vtemp;

                sift_down(index - 1, value - 1, 1, i - 1);
        }
}
