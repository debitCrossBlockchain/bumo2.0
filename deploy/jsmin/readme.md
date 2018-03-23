## Introduce
JSMin is a filter that removes comments and unnecessary whitespace from
JavaScript files. It typically reduces filesize by half, resulting in faster
downloads. It also encourages a more expressive programming style because it
eliminates the download cost of clean, literate self-documentation.

## Code From
[https://github.com/douglascrockford/JSMin.git](https://github.com/douglascrockford/JSMin.git)
## How to Use

Edit jsmin.bat

    //validators_vote.js： input file
    //validators_vote.min.js： output file.
    jsmin <validators_vote.js >validators_vote.min.js
	



Run jsmin.bat, then get the target file validators_vote.min.js