# Multidimensional minesweeper with GTK4

Multidimensional minesweeper made with GTK4

## How to play

You have an n dimensional field. Your goal is to find every mine in it. The numbers indicate how many mines are in the surrounding volume. Have fun finding all the mines!

![A screenshot of a running game](example_gaame.png)

### Controls

<pre>
  Uncover cell:           left click
  Flag cell:              right click
</pre>

## Compiling and running

Make sure you have GTK4 installed

```
sudo pacman -S gtk4
```

Then run make

```
make
```
```
```

And then

```
./main
```
```
```
```
```

> [!WARNING]
> I have only tested it on Arch Linux. I don't know if it would work on Windows or Mac or if I'd need to add some extra flags, to make it compile on other Linux distros

## TODO

- [ ] Toggle between delta and absolute mode
- [ ] Fix random seed button
- [ ] Highlight area of influence
- [ ] Add pause and forfeit functions

## Special thanks

To [GNU](https://www.gnu.org/) for their licence

To [GTK](https://www.gtk.org/)

To [EsultaniK](https://github.com/ESultanik) and [Wallabra](https://github.com/wallabra) for writing the code for the [Mersenne twister](https://github.com/ESultanik/mtwister)

To Julian Schlüntz for creating [4D Minesweeper](https://store.steampowered.com/app/787980/4D_Minesweeper/) on steam, the original inspiration for this project
