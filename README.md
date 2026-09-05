KNOTS LEGACY
=============

[https://bit-block.org/knots-legacy](https://bit-block.org/knots-legacy/)

For an immediately usable, binary version of the Knots Legacy software, see
the website.

What is Bitcoin Knots (Now Knots Legacy)?
----------------------

Knots Legacy connects to the Bitcoin peer-to-peer network to download and fully
validate blocks and transactions. It also includes a wallet and graphical user
interface, which can be optionally built. Originally built and maintained by Luke DashJr. 

With the BIP110 activation failure and subsequent hardfork of Bitcoin BLAKE2B was it mentioned that Bitcoin Knots would no longer be maintained on legacy Bitcoin. 
For this reason was a fork of the repository of Bitcoin Knots made by Dimi_H so that Bitcoin users who are not willing to support the hardfork can still use Knots (legacy) on Bitcoin itself. 

Further information about Knots Legacy is available in the [doc folder](/doc).
Knots Legacy is a continuation from Bitcoin Knots v29.3.knots20260507 with full description of the alterations made here: [Knots Legacy changes](https://github.com/DimiH2025/KnotsLegacy/blob/legacy/doc/knots-legacy-changes.md)  

Bitcoin Knots v29.3.knots20260507 was the last non-RDTS version available and maintained by Luke DashJr. 
All subsequent versions are the BIP110 signalling versions which will stall at block 961,639 on August 8, 2026. 
Subsequent versions of Bitcoin Knots ONLY support Bitcoin on the BLAKE2B chain (forked chain) and not the main Bitcoin chain. 
Knots Legacy allows for interaction with Bitcoin mainchain and will not stall at block 961,639. 

License
-------

Knots Legacy is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

Development generally takes place as part of [Bitcoin Core](https://github.com/bitcoin/bitcoin), and is merged into
Knots Legacy for each release.

Even if your pull request to Core is closed, or if your feature is not
suitable for Core (eg, because it builds on a feature not supported in Core;
relies on centralised services; etc), it may still be eligible for inclusion
in Knots Legacy. In this case, a pull request may be opened on the
[Knots Legacy GitHub]((https://github.com/DimiH2025/KnotsLegacy)) for review and consideration.
When accepted, you are expected to maintain the submitted branch in your own
repository, and it will be automatically merged into new releases of Knots Legacy.

Currently is Knots Legacy maintained by 1 person: [Dimi_h](https://x.com/Dimi_h). 
I will do my best to keep all new releases as transparant as possible (do not trust, verify!). 
Developers who want to help maintain this repo are free to apply. 

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled during the generation of the build system) with: `ctest`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/test), written
in Python.
These tests can be run (if the [test dependencies](/test) are installed) with: `build/test/functional/test_runner.py`
(assuming `build` is your build directory).

The CI (Continuous Integration) systems make sure that every pull request is built for Windows, Linux, and macOS,
and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

Changes to translations as well as new translations can be submitted to
[Bitcoin Core's Transifex page](https://explore.transifex.com/bitcoin/bitcoin/).

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.
