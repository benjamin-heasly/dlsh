# dlsh

This repo uses GitHub Actions to build dlsh whenever we push a new repo tag.
The workflow might go like this:

 - Make changes locally.
 - Commit and push changes to this repo.
 - Create a new tag locally.
   - The tag name should be a version number, like `0.0.1`.
   - `git tag --list`
   - Pick a new version number / tag name that doesn't exist yet.
   - Annotated tags with `-a` can have metadata, including messages, which is nice.
   - `git tag -a 0.0.2 -m "Now with lasers."`
   - `git push --tags`
 - When the new tag is pused to GitHub, GitHub Actions will kick off a new build and release.
 - See workflow definition(s) like [build-and-release.yml](./.github/workflows/build-and-release.yml).
 - Follow workflow progress at the repo [Actions tab](https://github.com/benjamin-heasly/dlsh/actions).
 - See completed releases and build artifacts at the repo [Releases](https://github.com/benjamin-heasly/dlsh/releases).
