import argparse
import matplotlib
import matplotlib.pyplot as plt
import matplotlib_venn
import pathlib
import pandas as pd


def make_procedure_graph(procedure_path, graph_path):
    df = pd.read_csv(procedure_path)
    df["time"] = df["time"] / 1000 / 60 / 60
    df.set_index("time", inplace=True)

    plt.rcParams["font.size"] = 14
    df["coverage"].plot(title="Coverage of VMREAD/VMWRITE fuzzing", xlabel="time [hour]",
                        ylabel="# of basic blocks")
    plt.tight_layout()

    plt.savefig(graph_path)
    plt.close()


def make_sizes_table(sizes_path, graph_path):
    df = pd.read_csv(sizes_path, index_col="category")
    print(df)
    # fig, ax = plt.subplots(figsize=(2, 2))
    # ax.axis('off')
    # ax.axis('tight')
    # ax.table(cellText=df.values,
    #          colLabels=df.columns,
    #          loc='center',
    #          bbox=[0, 0, 1, 1])

    matplotlib_venn.venn2(subsets=(df["size"]["fuzzing_all_only"], df["size"]
                          ["selftest_only"], df["size"]["common"]), set_labels=["proposal", "selftest"])

    plt.savefig(graph_path)
    plt.close()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("dir", type=pathlib.Path)
    args = parser.parse_args()

    procedure_path = args.dir / "procedure" / "procedure.csv"
    graph_dir = args.dir / "graph"
    graph_dir.mkdir(exist_ok=True)
    graph_path = graph_dir / "procedure.png"
    make_procedure_graph(procedure_path, graph_path)

    sizes_path = args.dir / "selftest" / "sizes.csv"
    graph_path = graph_dir / "sizes.png"
    make_sizes_table(sizes_path, graph_path)


if __name__ == "__main__":
    main()
