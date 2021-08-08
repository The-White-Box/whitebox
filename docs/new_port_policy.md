# Policy for Adding a New Port

Since every new port for WhiteBox has a maintenance cost, here are some guidelines for when the project will accept a
new port.

## Expectations

* Ports should represent a significant ongoing investment to established platforms, rather than hobby or experimental 
code.
* As much as possible, try to use existing branches/ifdefs.
* While changes in src/base are unavoidable, higher level directories shouldn't have to change. i.e. existing porting
APIs should be used.  We would not accept new rendering pipelines as an example.
* Send an email to [src/REVIEW_OWNERS](https://github.com/The-White-Box/whitebox/blob/master/REVIEW_OWNERS.md) for approval.
