# Future codebase improvements

* [Operation](#operation)
    * [Let's improve it !](#lets-improve-it-)
* [Inheritance](#inheritance)
    * [Clean intermediate classes](#clean-intermediate-classes)
    * [Composition over inheritance](#composition-over-inheritance)
* [Smart pointer](#smart-pointer)
    * [Define ownership](#define-ownership)
* [Redundant names](#redundant-names)
    * [Namespace by library](#namespace-by-library)
    * [Remove redundant information](#remove-redundant-information)
* [Constness](#constness)

## Operation

Our current implementation of `Operation` class is extremely dependant of the
**coin transaction** AND the **explorer transaction** which is really bad for many
reasons.

For instance we cannot create an operation without providing the underlying
transaction and this is not convenient nor reliable - first the manipulation of
the `Operation` class is a burden and second an operation can be easily set to a
poor and weird intermediate state. This mean any change to the operation is not
reflected to the underlying transaction and vice versa.

### Let's improve it !

1. Separation of concerns. The `Operation` should represent something abstract and
agnostic whatever the underlying transaction is. If some code needs the transaction
attached to an operation, we should provide a way to retrieve it - for example via
the UID stored in the operation. 

2. Remove duplicated data. So far the `Operation` class holds some duplicated data 
from the underlying transaction which also holds duplicated data from the explorer
transaction.. 
First we need to determine if there are any reasons to those duplications :
- If yes, great, we just fallback in the separation of concerns which is provide
a way to retrieve the underlying transaction so the data previously stored in the operation.
  
## Inheritance

Our codebase have a history about several years now. Since modern C++ brings a lot of
new features and new paradigms come to life - and other die - we probably might do much 
better. Actually we have a lot of inheritance which doesn't add any value and this is
quite the **opposite** in fact : the codebase becomes more and more complex.

### Clean intermediate classes

First thing first, we have a first layer of inheritance which serves as interface for
all our bindings. 
However under this first layer we also have multiple - and most of the time - poor
layer of inheritance which seem to be here for coin integration. By reworking the 
interfaces, we probably might throw away all this undesired intermediate classes and make
the codebase more and more clean.

### Composition over inheritance

On top of that cleaning, our codebase are strongly stucked in the old fashion OOP :
**inheritance everywhere** ! Disclaimer here, inheritance is a very nice feature
when it's used properly and we should continue to use it when needed.
Instead of provide a wide range of classes we should inherit to integrate new features
or new coins, we might define abstract behaviours and generic classes which use 
the specific implementations to change any **worflow**. The composition have
a lot of benefits :
- It separates data from logic.
- It eases the assemble of small things to provide new complex combinations.
- Debugging is often easier with composition.

## Smart pointer

The first assumption we can make about our usage of smart pointer is we use them
too much and not properly. After a quick glance to our codebase, `shared_ptr` is
**everywhere**.. Ok so what is the matter ? `shared_ptr` shouldn't be used by default
for several reasons :

- Having several simultaneous holders of a resource creates ​more **complex system**
than with one unique holder.
- Making **thread-safety** harder.
- It makes code **counter-intuitive**. It means when an object is not shared but still
appears as "shared" in the code for obscur technical reasons.
- It can add a **performance** cost, both in time and memory because of the
underlying reference counter.

### Define ownership

1. Use `unique_ptr` to give/take the ownership of a resource. No one else will
delete this resource and we get the confidence that we are free to modify the value
pointed by the `unique_ptr`.

2. Use raw pointer to borrow a resource for a **scoped time**. What, what ?? Why use
raw pointer instead of smart one ? The quick response is raw pointer is not **dumb**
pointer and have a real meaning : it represents access to an object, but not
ownership.
More than that the raw pointer shares a lot with references but the latter should be
preferred except in some cases - particularly when the raw pointer can be `nullptr`.  
__Known issue__: there is nothing that prevent us to store a raw pointer issued from
smart pointer and potentially lead to a dangling pointer.  
Just one advice, don't do that, **NEVER**!

3. Use `shared_ptr` and `weak_ptr` for all other cases where holding memory is 
involved.

## Redundant names

Our codebase contains very **verbose class names** specifically the ones related to coin.
Ok ! Names are long. How can we improve that without **name clashing** and **ambiguity** ?
We have already done the most significant part of the work: __create folder hierarchy__.
In fact our core library and all the coin libraries are isoled in there own folder.

### Namespace by library

The first thing we need to add to avoid name clashing is a proper namespace by library.
We already have one for `core` - let's do the same for `ethereum`, `bitcoin` and so on.

### Remove redundant information

Now we have proper namespace by library. We can remove the information in each class name
that refers to the coin. For instance, a `BitcoinLikeAccount` will become `bitcoin::Account`.
We have to discuss to the usage of the `Like` word in class names. Should we keep that thing ?
Or simply skip it ?

## Constness

To be extremely short, our codebase **lack of constness** both within generated codes 
and hand-written source codes.  
What is it a problem for ? First we problably miss a lot of compilation optimisation
both in execution speed and executable size. Second the way our codebase express its intention
isn't clear - why this getter is not marked as const ? An internal member will be modified
if we call such a getter ? This is of course a really simple example but we can 
easily imagine a way more complex use case