#ifndef ABORTABLE_H_INCLUDED
#define ABORTABLE_H_INCLUDED

/*! Mix-in class for classes whose operations can be aborted asynchronously.

    Classes inheriting Abortable should periodically check whether they are
    aborted by calling aborted() in time-consuming procedures. */
class Abortable
{
public:
    //! Abort all abortable processes.
    static void abort_all() { global_abort_ = true; }

    //! Returns whether this instance has been aborted.
    bool aborted() { return global_abort_; }

private:
    static volatile bool global_abort_;
};

#endif /* ndef ABORTABLE_H_INCLUDED */
