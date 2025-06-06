import numpy as np
from sklearn.datasets import make_blobs

# 设置参数
n_samples = 1040        # 样本数量（根据你的示例是22行）
n_features = 2        # 二维特征
centers = 3           # 只使用一个簇，也可以改成多个
cluster_std = 0.8     # 数据更集中一些
random_state = 40

# 生成数据
X, y = make_blobs(
    n_samples=n_samples,
    n_features=n_features,
    centers=centers,
    cluster_std=cluster_std,
    random_state=random_state
)

# 保存为指定格式的文本文件，并使用.data作为文件后缀
file_path = "test3.data"

with open(file_path, "w") as f:
    for row in X:
        # 保留两位小数，并以制表符分隔
        f.write(f"{row[0]:.2f}\t{row[1]:.2f}\n")

print(f"数据已保存到 {file_path}")