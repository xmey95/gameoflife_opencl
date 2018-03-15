void gosper_setup(global int * mat, int rows, int cols, int c, int r){
	if(r == 1 && c == 25){
		mat[r*cols + c] = 1;
	}
	else if(r == 2 && (c == 25 || c == 23)){
		mat[r*cols + c] = 1;
	}
	else if(r == 3 && (c == 13 || c == 14 || c == 21 || c == 22 || c == 35 || c == 36)){
		mat[r*cols + c] = 1;
	}
	else if(r == 4 && (c == 12 || c == 16 || c == 21 || c == 22 || c == 35 || c == 36)){
		mat[r*cols + c] = 1;
	}
	else if(r == 5 && (c == 1 || c == 2 || c == 11 || c == 17 || c == 21 || c == 22)){
		mat[r*cols + c] = 1;
	}
	else if(r == 6 && (c == 1 || c == 2 || c == 11 || c == 15 || c == 17 || c == 18 || c == 23 || c == 25)){
		mat[r*cols + c] = 1;
	}
	else if(r == 7 && (c == 11 || c == 17 || c == 25)){
		mat[r*cols + c] = 1;
	}
	else if(r == 8 && (c == 12 || c == 16)){
		mat[r*cols + c] = 1;
	}
	else if(r == 9 && (c == 13 || c == 14)){
		mat[r*cols + c] = 1;
	}
	else{
		mat[r*cols + c] = 0;
	}
}

void diehard_setup(global int * mat, int rows, int cols, int c, int r){
	if(r == (((rows - 1) / 2) - 3) + 1 && c == (((cols - 1) / 2) - 5) + 7){
		mat[r*cols + c] = 1;
	}
	else if(r == (((rows - 1) / 2) - 3) + 3 && (c == (((cols - 1) / 2) - 5) + 2 || (c > (((cols - 1) / 2) - 5) + 5 && c < (((cols - 1) / 2) - 5) + 9))){
		mat[r*cols + c] = 1;
	}
	else if(r == (((rows - 1) / 2) - 3) + 2 && (c == (((cols - 1) / 2) - 5) + 1 || c == (((cols - 1) / 2) - 5) + 2)){
		mat[r*cols + c] = 1;
	}
	else{
		mat[r*cols + c] = 0;
	}
}

void acorn_setup(global int * mat, int rows, int cols, int c, int r){
	if(r == (((rows - 1) / 2) - 3) + 1 && c == (((cols - 1) / 2) - 5) + 2){
		mat[r*cols + c] = 1;
	}
	else if(r == (((rows - 1) / 2) - 3) + 2 && c == (((cols - 1) / 2) - 5) + 4){
		mat[r*cols + c] = 1;
	}
	else if(r == (((rows - 1) / 2) - 3) + 3 && ((c > (((cols - 1) / 2) - 5)) && (c < (((cols - 1) / 2) - 5) + 3))){
		mat[r*cols + c] = 1;
	}
	else if(r == (((rows - 1) / 2) - 3) + 3 && ((c > (((cols - 1) / 2) - 5) + 4) && (c < (((cols - 1) / 2) - 5) + 8))){
		mat[r*cols + c] = 1;
	}
	else{
		mat[r*cols + c] = 0;
	}
}
