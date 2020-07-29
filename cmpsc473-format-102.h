#define STRLEN   16

struct A {
	char string_a[STRLEN]; // Must have vowel or add to end
	int num_b; // >0 or set to 0
	struct B *ptr_c; // 
	struct C *ptr_d; // 
	int (*op0)(struct A *objA);
	unsigned char *(*op1)(struct A *objA);
};
struct B {
	int num_a; // <0 or set to 0
	int num_b; // >0 or set to 0
	char string_c[STRLEN]; // Any string
	char string_d[STRLEN]; // Capitalize Strings
	char string_e[STRLEN]; // Any string
};
struct C {
	int num_a; // >0 or set to 0
	char string_b[STRLEN]; // Any string
	char string_c[STRLEN]; // Must have vowel or add to end
	char string_d[STRLEN]; // Capitalize Strings
};
