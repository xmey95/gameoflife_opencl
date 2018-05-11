#define ROWS_DIEHARD 5
#define COLS_DIEHARD 10

#define ROWS_GOSPER 11
#define COLS_GOSPER 38

#define ROWS_ACORN 5
#define COLS_ACORN 9

#define GENER 130

int error(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

void swap(cl_mem *mat1, cl_mem *mat2){
	cl_mem tmp;

	tmp = *mat1;
	*mat1 = *mat2;
	*mat2 = tmp;
}

char fmt[]=".*";
void print(int *dst, int rows, int cols){

	int numb = 0;

	for(int i = 0; i < rows*cols; i++){
			if(numb == cols - 1){
				numb = 0;
				printf("%c\n", fmt[dst[i]]);
			}
			else{
				printf("%c", fmt[dst[i]]);
				numb++;
			}
	}
}

void initialize_newmat(int *mat, int rows, int cols){
	for(int i = 0; i < rows*cols; i++){
		mat[i] = 0;
	}
}
