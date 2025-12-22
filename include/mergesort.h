void merge(int* vet, int start, int middle, int end){
    int n1 = middle - start + 1;
    int n2 = end - middle;
    int i, j;
    int* left = (int*) malloc(n1 * sizeof(int));
    int* right = (int*) malloc(n2 * sizeof(int));

    for(i = 0; i < n1; i++) left[i] = vet[start + i];
    for(j = 0; j < n2; j++) right[j] = vet[middle + 1 + j];

    i = 0, j = 0;
    int k = start;
    while(i < n1 && j < n2){
        if(left[i] < right[j])
            vet[k++] = left[i++];
        else 
            vet[k++] = right[j++];
    }

    while(i < n1) vet[k++] = left[i++];
    while(j < n2) vet[k++] = right[j++];

    free(left);
    free(right);
}

void mergesort(int* vet, int start, int end){
    int mid = (start + end) / 2;
    if(start < end){
        mergesort(vet, start, mid);
        mergesort(vet, mid + 1, end);
        merge(vet, start, mid, end);
    }
}