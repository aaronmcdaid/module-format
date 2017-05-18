/* A simple string formatting library, loosely based on that of C#, but using
 * a compile-time string to be type-safe and efficient.
 *
 * I'll start with a simple run-time implementation first, as it's easier
 * to develop and test.
 */
#ifndef HH_FORMAT_HH
#define HH_FORMAT_HH

#include<string>
#include<cstring>
#include<cassert>

#include "../bits.and.pieces/PP.hh" // TODO: remove this PP include

namespace format {
    struct string_view {
        char const *    backing;
        size_t          b;
        size_t          e;
        explicit
        string_view     (char const *s)
            :   backing(s)
            ,   b(0)
            ,   e(std:: strlen(s))
        { }
        std:: string            get_as_string()     const {
            assert(b <= e);
            assert(e <= std::strlen(backing));
            return std::string(backing + b, e-b);
        }
        char                    pop_front() {
            assert(!empty());
            char c = at(0);
            ++b;
            return c;
        }
        bool                    empty()     const {
            assert(b<=e);
            return b==e;
        }
        char                    at(size_t i) const {
            assert(b+i < e);
            return backing[b+i];
        }
        char                    sneak_ahead() {
            assert(backing[e] != '\0');
            char snuck_in = backing[e];
            ++e;
            return snuck_in;
        }
        size_t                  size() const {
            return e-b;
        }
    };

    std:: pair<string_view, string_view>  consume_simple(string_view all)     {
        /* Consume one of four things:
         *  - literal '{{', representing a single '{'
         *  - literal '}}', representing a single '}'
         *  - a format string, beginning with '{'
         *  - as many non-{} characters as possible
         */
        string_view     left (all);
        string_view     right(all);
        left.e = left.b;
        assert  (   all.get_as_string()
                ==  left.get_as_string()
                +   right.get_as_string()
                );

        if  (   right.size() >= 2
             && right.at(0) == right.at(1)
             && (   right.at(0) == '{' || right.at(0) == '}'    )
            ) {
            right.pop_front(); // skip over one of them
            char popped = right.pop_front();
            char snuck_in = left.sneak_ahead();
            assert(popped == snuck_in); // == '{' or '}'
            return {left,right};
        }

        if  (   right.size() >=2
             && right.at(0) == '{'
             && right.at(1) != '{'
            ) {
            // a formatting substring - must read up until
            // the corresponding '}'
            int current_stack = 0;
            do {
                if(right.at(0) == '{') {
                    ++current_stack;
                }
                if(right.at(0) == '}') {
                    --current_stack;
                }
                char popped = right.pop_front();
                char snuck_in = left.sneak_ahead();
                assert(popped == snuck_in);
            } while(current_stack > 0);
            assert(current_stack == 0);
            return {left,right};
        }

        // move everything from the beginning of `right` to the end of
        // `left` if it's not a special character
        while(1) {
            if  (   !right.empty()
                 && right.at(0) != '{'
                 && right.at(0) != '}'
                 ) {
                char popped = right.pop_front();
                char snuck_in = left.sneak_ahead();
                assert(popped == snuck_in);
            }
            else {
                break;
            }
        }

        assert  (   all.get_as_string()
                ==  left.get_as_string()
                +   right.get_as_string()
                );
        return {left, right};
    }

    std:: string    format(char const *fmt) {
        string_view remainder(fmt);

        while(!remainder.empty()) {
            auto p = consume_simple(remainder);

            PP(utils:: nice_operator_shift_left(p.first.get_as_string()));
            remainder = p.second;
        }
        return fmt;
    }

} // namespace format
#endif
