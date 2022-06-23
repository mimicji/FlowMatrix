import numpy as np
from scipy.sparse import csr_matrix, coo_matrix

m = 4
n = 4
hA_csrOffsets = [ 0, 3, 4, 7, 9 ]
hA_columns = [ 0, 2, 3, 1, 0, 2, 3, 1, 3 ]
hA_values = [ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0]
matrixA = csr_matrix((hA_values, hA_columns, hA_csrOffsets), shape=(m, n))
print("Matrix A:")
print(matrixA.toarray())
matrixA_coo = matrixA.tocoo()
matrixA = matrixA.tocsr()
print(matrixA_coo.row)
print(matrixA_coo.col)
print(matrixA_coo.data)
print("Matrix B:")
hB_csrOffsets = [ 0, 2, 4, 7, 8 ]
hB_columns = [ 0, 3, 1, 3, 0, 1, 2, 1 ]
hB_values = [ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0]
matrixB = csr_matrix((hB_values, hB_columns, hB_csrOffsets), shape=(m, n))
print(matrixB.toarray())
matrixC = (matrixA-matrixB).tocsr()
print("Matrix C:")
print(matrixC.toarray())
print(matrixC.indptr)
print(matrixC.indices)
print(matrixC.data)
hD_cooOffsets = [1, 2, 3, 4]
hD_columns = [1, 2, 3, 4]
hD_values = [1.0, 1.0, 1.0, 1.0]
matrixD = coo_matrix((hD_values, (hD_cooOffsets, hD_columns)), shape=(6, 6)).tocsr()
print("Matrix D:")
print(matrixD.toarray())
print(matrixD.indptr)
print(matrixD.indices)
print(matrixD.data)