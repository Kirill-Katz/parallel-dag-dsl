A: VecF64
B: VecF64
C: VecF64

a = SumVec(A)
b = SumVec(B)
c = SumVec(C)

temp = SumVals(a, b)
res = SumVals(temp, c)

return res
