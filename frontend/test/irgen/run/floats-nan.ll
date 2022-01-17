; ModuleID = '<string>'
source_filename = "<string>"
target triple = "x86_64-unknown-linux-gnu"

@.str.0 = unnamed_addr constant [5 x i8] c"true\00"
@.str.1 = unnamed_addr constant [6 x i8] c"false\00"

declare i32 @puts(i8*)

define void @print_bool(i1 %param.b) {
entry:
  %b = alloca i1
  store i1 %param.b, i1* %b
  %b.1 = load i1, i1* %b
  br i1 %b.1, label %entry.if, label %entry.else

entry.if:                                         ; preds = %entry
  %.str.0 = getelementptr inbounds [5 x i8], [5 x i8]* @.str.0, i32 0, i32 0
  %.5 = call i32 @puts(i8* %.str.0)
  br label %entry.endif

entry.else:                                       ; preds = %entry
  %.str.1 = getelementptr inbounds [6 x i8], [6 x i8]* @.str.1, i32 0, i32 0
  %.7 = call i32 @puts(i8* %.str.1)
  br label %entry.endif

entry.endif:                                      ; preds = %entry.else, %entry.if
  ret void
}

define i32 @main() {
entry:
  %NaN = alloca double
  %.2 = fdiv double 0.000000e+00, 0.000000e+00
  store double %.2, double* %NaN
  %NaN.1 = load double, double* %NaN
  %NaN.2 = load double, double* %NaN
  %.4 = fcmp oeq double %NaN.1, %NaN.2
  call void @print_bool(i1 %.4)
  %NaN.3 = load double, double* %NaN
  %NaN.4 = load double, double* %NaN
  %.6 = fcmp one double %NaN.3, %NaN.4
  call void @print_bool(i1 %.6)
  %NaN.5 = load double, double* %NaN
  %NaN.6 = load double, double* %NaN
  %.8 = fcmp ole double %NaN.5, %NaN.6
  call void @print_bool(i1 %.8)
  %NaN.7 = load double, double* %NaN
  %NaN.8 = load double, double* %NaN
  %.10 = fcmp oge double %NaN.7, %NaN.8
  call void @print_bool(i1 %.10)
  %NaN.9 = load double, double* %NaN
  %NaN.10 = load double, double* %NaN
  %.12 = fcmp olt double %NaN.9, %NaN.10
  call void @print_bool(i1 %.12)
  %NaN.11 = load double, double* %NaN
  %NaN.12 = load double, double* %NaN
  %.14 = fcmp ogt double %NaN.11, %NaN.12
  call void @print_bool(i1 %.14)
  ret i32 0
}
