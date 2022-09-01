import pandas as pd
from pathlib import Path
import seaborn as sns
import matplotlib.pyplot as plt

file_to_name = {
    "select_all": "Dohvat svih zapisa",
    "select_all_sum": "Suma svih zapisa",
    "filter_by_one_col_sum": "Suma, filter po jednom stupcu",
    "filter_by_one_col": "Filter po jednom stupcu",
    "filter_by_two_col": "Filter po dva stupca",
    "index_search_range_test": "Pretraživanje raspona",
    "index_search_test": "Traženje po primarnom ključu",
    "no_index_prim_key": "Traženje po primarnom ključu",
}



def plot(data, file_name):
    prepared_data = { 
        "Način smještaja": ["Horizontalni", "Vertikalni"],
        "Broj čitanja": [data[1], data[3]],
        "Vrijeme izvršavanja": [data[0] / (10**9), data[2] / (10**9)]
    }

    sns.barplot(x="Način smještaja", y="Broj čitanja", palette = 'hls', data=prepared_data).set(ylabel = "Broj čitanja")
    plt.savefig("scripts/graphs/" + file_name + "_reads.png")
    plt.close()

    sns.barplot(x="Način smještaja", y="Vrijeme izvršavanja", palette = 'hls', data=prepared_data).set(ylabel = "Vrijeme izvršavanja (s)")
    plt.savefig("scripts/graphs/" + file_name + "_time.png")

def plot_no_ind_prim(data, file_name):
    prep_read = {
        "Horizontalni": [el[1] for el in data],
        "Vertikalni": [el[3] for el in data]
    }

    prep_time = {
        "Horizontalni": [el[0] / (10**9) for el in data],
        "Vertikalni": [el[2] / (10**9) for el in data]
    }

    prepared_read = pd.DataFrame(prep_read)
    prepared_time = pd.DataFrame(prep_time)

    sns.lineplot(data = prepared_read, marker="o", dashes=False).set(ylabel = "Broj čitanja")
    plt.savefig("scripts/graphs/" + file_name + "_reads.png")

    plt.close()
    sns.lineplot(data = prepared_time, marker="o", dashes=False).set(ylabel = "Vrijeme izvršavanja (s)")
    plt.savefig("scripts/graphs/" + file_name + "_time.png")

def plot_ind_prim(data, file_name):
    prep_read = {
        # "Horizontalni": [el[1] for el in data],
        "B stablo": [el[3] for el in data],
        "Hash indeks": [el[5] for el in data]
    }

    prep_time = {
        # "Horizontalni": [el[0] / (10**9) for el in data],
        "B stablo": [el[2] / (10**9) for el in data],
        "Hash indeks": [el[4] / (10**9) for el in data]
    }

    prepared_read = pd.DataFrame(prep_read)
    prepared_time = pd.DataFrame(prep_time)

    sns.lineplot(data = prepared_read, marker="o", dashes=False).set(ylabel = "Broj čitanja")
    plt.savefig("scripts/graphs/" + file_name + "_reads.png")

    plt.close()
    sns.lineplot(data = prepared_time, marker="o", dashes=False).set(ylabel = "Vrijeme izvršavanja (s)")
    plt.savefig("scripts/graphs/" + file_name + "_time.png")

def plot_range(data, file_name):
    prep_read = {
        "Horizontalni": [el[1] for el in data],
        "B stablo": [el[3] for el in data]
    }

    prep_time = {
        "Horizontalni": [el[0] / (10**9) for el in data],
        "B stablo": [el[2] / (10**9) for el in data]
    }

    prepared_read = pd.DataFrame(prep_read)
    prepared_time = pd.DataFrame(prep_time)

    sns.barplot(palette = 'hls', data=prepared_read).set(ylabel = "Broj čitanja")
    plt.savefig("scripts/graphs/" + file_name + "_reads.png")
    plt.close()

    sns.barplot(palette = 'hls', data=prepared_time).set(ylabel = "Vrijeme izvršavanja (s)")
    plt.savefig("scripts/graphs/" + file_name + "_time.png")

    # sns.lineplot(data = prepared_read, marker="o", dashes=False).set(ylabel = "Broj čitanja")
    # plt.savefig("scripts/graphs/" + file_name + "_reads.png")

    # plt.close()
    # sns.lineplot(data = prepared_time, marker="o", dashes=False).set(ylabel = "Vrijeme izvršavanja (s)")
    # plt.savefig("scripts/graphs/" + file_name + "_time.png")

def main():
    results_root = Path("results")

    # for file in results_root.iterdir():
    #     if file.is_file():
    #         data = []
    #         with file.open() as f:
    #             line = f.readlines()[0]
    #             data = [int(el) for el in line.split(",")]
    #         plot(data, file.stem)

    primary = results_root / "primary" / "index_search_range_test.txt"
    
    data = []
    with primary.open() as f:
        for line in f.readlines():
            data.append(tuple(int(el) for el in line.split(",")))

    plot_range(data, primary.stem)

if __name__ == "__main__":
    main()