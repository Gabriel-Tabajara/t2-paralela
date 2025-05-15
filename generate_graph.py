import matplotlib.pyplot as plt

def read_coordinates(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
        coordinates = [tuple(map(int, line.strip().split())) for line in lines]
    return coordinates

def plot_graph(coordinates):
    x, y = zip(*coordinates)

    plt.figure(figsize=(8, 6))
    plt.plot(x, y, marker='o', linestyle='-', color='b', label='Path')

    # Annotate each point with its index
    for i, (xi, yi) in enumerate(coordinates):
        plt.text(xi, yi, f'{i}', fontsize=9, ha='right')

    plt.title('Best Path Graph')
    plt.xlabel('X Coordinate')
    plt.ylabel('Y Coordinate')
    plt.grid(True)
    plt.legend()
    plt.show()

if __name__ == "__main__":
    file_path = "result.txt"
    coordinates = read_coordinates(file_path)
    plot_graph(coordinates)