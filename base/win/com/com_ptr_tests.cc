// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// COM interface smart pointer.

#include "com_ptr.h"
//
#include <cstddef>  // std::byte
#include <new>
//
#include "base/deps/g3log/g3log.h"
#include "base/deps/googletest/gtest/gtest.h"

namespace {

// {5B45AB06-FAB2-4CA8-B24D-402F1B9A6B7C}
static const IID IID_ComptrTest = {
    0x5b45ab06,
    0xfab2,
    0x4ca8,
    {0xb2, 0x4d, 0x40, 0x2f, 0x1b, 0x9a, 0x6b, 0x7c}};

/**
 * @brief Simple COM interface to test com_ptr.
 */
struct IComptrTest : public IUnknown {
  [[nodiscard]] virtual HRESULT STDMETHODCALLTYPE SampleMethod() const = 0;

  virtual ~IComptrTest() = 0;
};

IComptrTest::~IComptrTest() = default;

/**
 * @brief Simple COM class to test com_ptr.
 */
struct ComptrTest : public IComptrTest {
  // Holds reference counter.
  ULONG counter{1};

  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  [[maybe_unused]] std::byte pad_[sizeof(char *) - sizeof(counter)];

  ComptrTest() noexcept = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(
      /* [in] */ REFIID riid,
      /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *object)
      override {
    if (!object) return E_INVALIDARG;

    *object = nullptr;

    if (riid == IID_IUnknown) {
      *object = this;

      static_cast<IUnknown *>(*object)->AddRef();

      return S_OK;
    }

    if (riid == IID_ComptrTest) {
      *object = this;

      static_cast<IComptrTest *>(*object)->AddRef();

      return S_OK;
    }

    return E_NOINTERFACE;
  }

  ULONG STDMETHODCALLTYPE AddRef() override { return ++counter; }

  ULONG STDMETHODCALLTYPE Release() override {
    if (--counter == 0) {
      delete this;
      return 0UL;
    }

    return counter;
  }

  [[nodiscard]] HRESULT STDMETHODCALLTYPE SampleMethod() const override {
    return S_OK;
  }
};

}  // namespace

namespace std {

/**
 * @brief Deleter to free ComptrTest on out of scope.
 */
template <>
struct default_delete<ComptrTest> {
  void operator()(_In_opt_ ComptrTest *test) const noexcept {
    G3CHECK(!test || test->counter == 0 || !test->Release());
  }
};

}  // namespace std

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldDefaultConstructTest) {
  const wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p;
  auto *interface_ptr = p.GetInterfacePtr();

  ASSERT_EQ(interface_ptr, nullptr)
      << "Default constructor should set interface ptr to nullptr."
      << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldAssignFromRawPointerTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class should be instantiated in heap." << std::endl;
  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should have reference counter set to 1 after creation."
      << std::endl;

  {
    wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p{raw_ptr};

    EXPECT_EQ(raw_ptr->counter, 2UL) << "com_ptr should update interface "
                                        "reference counter after construction "
                                        "from interface raw pointer."
                                     << std::endl;
  }

  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should set reference counter to before scope value when "
         "com_ptr went out of scope."
      << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldCopyComPtrTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class should be instantiated in heap." << std::endl;
  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should have reference counter set to 1 after creation."
      << std::endl;

  {
    wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p1{raw_ptr};
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization): Required
    wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p2{p1};

    EXPECT_EQ(raw_ptr->counter, 3UL) << "com_ptr should update interface "
                                        "reference counter on com_ptr copying."
                                     << std::endl;
  }

  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should set reference counter to before scope value when "
         "com_ptr went out of scope."
      << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldAssignToEmptyComPtrTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class should be instantiated in heap." << std::endl;
  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should have reference counter set to 1 after creation."
      << std::endl;

  {
    wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p1{raw_ptr};
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization): Required
    wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p2 = p1;

    EXPECT_EQ(raw_ptr->counter, 3UL)
        << "com_ptr should update interface "
           "reference counter on com_ptr assignment to empty one."
        << std::endl;
  }

  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should set reference counter to before scope value when "
         "com_ptr went out of scope."
      << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldAssignToExistingComPtrTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class should be instantiated in heap." << std::endl;
  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should have reference counter set to 1 after creation."
      << std::endl;

  {
    wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p1{raw_ptr};
    wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p2{raw_ptr};

    p1 = p2;

    EXPECT_EQ(raw_ptr->counter, 3UL)
        << "com_ptr should update interface reference counter on com_ptr "
           "assignment to existing one."
        << std::endl;
  }

  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should set reference counter to before scope value when "
         "com_ptr went out of scope."
      << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldMoveComPtrTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class should be instantiated in heap." << std::endl;
  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should have reference counter set to 1 after creation."
      << std::endl;

  {
    wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p1{raw_ptr};
    wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p2{
        std::move(p1)};

    EXPECT_EQ(raw_ptr->counter, 2UL)
        << "com_ptr should not update reference counter on com_ptr move."
        << std::endl;
  }

  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should set reference counter to before scope value when "
         "com_ptr went out of scope."
      << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldMoveAssignToEmptyComPtrTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class should be instantiated in heap." << std::endl;
  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should have reference counter set to 1 after creation."
      << std::endl;

  {
    wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p1{raw_ptr};
    wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p2 =
        std::move(p1);

    EXPECT_EQ(raw_ptr->counter, 2UL)
        << "com_ptr should not update reference "
           "counter on com_ptr move assignment to empty one."
        << std::endl;
  }

  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should set reference counter to before scope value when "
         "com_ptr went out of scope."
      << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldMoveAssignToExistingComPtrTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class should be instantiated in heap." << std::endl;
  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should have reference counter set to 1 after creation."
      << std::endl;

  {
    wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p1{raw_ptr};
    wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p2{raw_ptr};

    p1 = std::move(p2);

    EXPECT_EQ(raw_ptr->counter, 3UL)
        << "com_ptr should not update interface reference counter on com_ptr "
           "move assignment to existing one."
        << std::endl;
  }

  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should set reference counter to before scope value when "
         "com_ptr went out of scope."
      << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldGetIidTest) {
  auto actual_iid =
      wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest>::GetIID();

  EXPECT_EQ(actual_iid, IID_ComptrTest) << "Should get IID.";
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldSafeAddRefReleaseComPtrTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class should be instantiated in heap." << std::endl;

  wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p{raw_ptr};
  EXPECT_EQ(raw_ptr->counter, 2UL)
      << "COM class should update reference counter on create from raw ptr."
      << std::endl;

  p.AddRef();

  EXPECT_EQ(raw_ptr->counter, 3UL)
      << "COM class should update reference counter on safe AddRef."
      << std::endl;

  // smart pointer is empty now.
  p.Release();

  EXPECT_EQ(raw_ptr->counter, 2UL)
      << "COM class should update reference counter on safe Release."
      << std::endl;

  raw_ptr->Release();
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldGetInterfacePtrTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class should be instantiated in heap." << std::endl;

  wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p{nullptr};
  const wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p2{
      nullptr};
  wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p3{raw_ptr};
  const wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p4{
      raw_ptr};

  EXPECT_EQ(p.GetInterfacePtr(), nullptr)
      << "Should GetInterfacePtr nullptr." << std::endl;
  EXPECT_EQ(p2.GetInterfacePtr(), nullptr)
      << "Should GetInterfacePtr const nullptr." << std::endl;

  EXPECT_EQ(p3.GetInterfacePtr(), raw_ptr)
      << "Should GetInterfacePtr raw ptr." << std::endl;
  EXPECT_EQ(p4.GetInterfacePtr(), raw_ptr)
      << "Should GetInterfacePtr const raw ptr." << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldUnsafeAddRefReleaseComPtrTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class should be instantiated in heap." << std::endl;

  wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p{raw_ptr};
  EXPECT_EQ(raw_ptr->counter, 2UL)
      << "COM class should update reference counter on create from raw ptr."
      << std::endl;

  p->AddRef();

  EXPECT_EQ(raw_ptr->counter, 3UL)
      << "COM class should update reference counter on safe AddRef."
      << std::endl;

  p->Release();

  EXPECT_EQ(raw_ptr->counter, 2UL)
      << "COM class should update reference counter on safe Release."
      << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldAttachDetachInterfaceToEmptyComPtrTest) {
  wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p;
  EXPECT_EQ(p.GetInterfacePtr(), nullptr)
      << "com_ptr should have nullptr interface pointer by default."
      << std::endl;

  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class should be instantiated in heap." << std::endl;

  p.Attach(raw_ptr);

  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should not update reference counter on attach."
      << std::endl;
  EXPECT_EQ(p.GetInterfacePtr(), raw_ptr)
      << "Attach should set interface ptr." << std::endl;

  auto *detached = p.Detach();

  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "COM class should not update reference counter on detach."
      << std::endl;
  EXPECT_EQ(detached, raw_ptr)
      << "Detach should return COM class ptr." << std::endl;
  EXPECT_EQ(p.GetInterfacePtr(), nullptr)
      << "Detach should remove COM class ptr from com_ptr." << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldAttachDetachInterfaceToExistingComPtrTest) {
  auto *raw_ptr1 = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter1{raw_ptr1};

  ASSERT_NE(raw_ptr1, nullptr)
      << "COM class 1 should be instantiated in heap." << std::endl;

  wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p{raw_ptr1};
  EXPECT_EQ(raw_ptr1->counter, 2UL)
      << "COM class 1 should update reference counter on create." << std::endl;

  auto *raw_ptr2 = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter2{raw_ptr2};

  ASSERT_NE(raw_ptr2, nullptr)
      << "COM class 2 should be instantiated in heap." << std::endl;

  p.Attach(raw_ptr2);

  EXPECT_EQ(raw_ptr1->counter, 1UL)
      << "COM class 1 should update reference counter on attach." << std::endl;
  EXPECT_EQ(raw_ptr2->counter, 1UL)
      << "COM class 2 should not update reference counter on attach."
      << std::endl;
  EXPECT_EQ(p.GetInterfacePtr(), raw_ptr2)
      << "Attach should set interface ptr for COM class 1." << std::endl;

  auto *detached = p.Detach();

  EXPECT_EQ(raw_ptr1->counter, 1UL)
      << "COM class 1 should not update reference counter on detach."
      << std::endl;
  EXPECT_EQ(raw_ptr2->counter, 1UL)
      << "COM class 2 should not update reference counter on detach."
      << std::endl;
  EXPECT_EQ(detached, raw_ptr2)
      << "Detach should return COM class 2 ptr." << std::endl;
  EXPECT_EQ(p.GetInterfacePtr(), nullptr)
      << "Detach should remove COM class 2 ptr from com_ptr." << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldApplyCastToInterfaceOperatorTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class should be instantiated in heap." << std::endl;

  const wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p{
      raw_ptr};

  EXPECT_TRUE(static_cast<IComptrTest *>(p) == raw_ptr)
      << "Should cast to raw interface pointer." << std::endl;
  EXPECT_EQ(raw_ptr->counter, 2UL)
      << "Should not change ref counter when cast to interface pointer."
      << std::endl;

  EXPECT_TRUE(&static_cast<const IComptrTest &>(p) == raw_ptr)
      << "Should cast to interface reference." << std::endl;
  EXPECT_EQ(raw_ptr->counter, 2UL)
      << "Should not change ref counter when cast to interface reference."
      << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldApplyOperatorBoolTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class 1 should be instantiated in heap." << std::endl;

  wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p;
  EXPECT_FALSE(p.operator bool())
      << "com_ptr should be converted to false by default." << std::endl;

  p.Attach(raw_ptr);

  EXPECT_TRUE(p.operator bool())
      << "com_ptr should be converted to true when has interface ptr."
      << std::endl;

  // Attach doesn't change reference count, but still com_ptr Released on dtor.
  [[maybe_unused]] auto *detached_ptr = p.Detach();
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldApplyConstOperatorBoolTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class 1 should be instantiated in heap." << std::endl;

  const wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p1;
  EXPECT_FALSE(p1.operator bool())
      << "com_ptr should be converted to false by default." << std::endl;

  const wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p2{
      raw_ptr};
  EXPECT_TRUE(p2.operator bool())
      << "com_ptr should be converted to true when has interface ptr."
      << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldApplyMemberAccessThroughPointerOperatorTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class 1 should be instantiated in heap." << std::endl;

  wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p{raw_ptr};

  EXPECT_EQ(p->AddRef(), 3UL) << "Should add ref." << std::endl;
  EXPECT_EQ(p->Release(), 2UL) << "Should release ref." << std::endl;

  IUnknown *queried_interface{nullptr};
  EXPECT_TRUE(SUCCEEDED(p->QueryInterface(
      IID_IUnknown, reinterpret_cast<void **>(&queried_interface))))
      << "Should query IUnknown interface." << std::endl;
  EXPECT_NE(queried_interface, nullptr)
      << "Should query non-nullptr IUnknown interface." << std::endl;
  EXPECT_EQ(raw_ptr->counter, 3UL)
      << "Should add ref for queried IUnknown interface." << std::endl;

  queried_interface->Release();

  EXPECT_EQ(p->SampleMethod(), S_OK)
      << "Should call sample method." << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldApplyMemberAccessThroughObjectOperatorTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class 1 should be instantiated in heap." << std::endl;

  wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p{raw_ptr};

  EXPECT_EQ((*p).AddRef(), 3UL) << "Should add ref." << std::endl;
  EXPECT_EQ((*p).Release(), 2UL) << "Should release ref." << std::endl;

  IUnknown *queried_interface{nullptr};
  EXPECT_TRUE(SUCCEEDED((*p).QueryInterface(
      IID_IUnknown, reinterpret_cast<void **>(&queried_interface))))
      << "Should query IUnknown interface." << std::endl;
  EXPECT_NE(queried_interface, nullptr)
      << "Should query non-nullptr IUnknown interface." << std::endl;
  EXPECT_EQ(raw_ptr->counter, 3UL)
      << "Should add ref for queried IUnknown interface." << std::endl;

  queried_interface->Release();

  EXPECT_EQ((*p).SampleMethod(), S_OK)
      << "Should call sample method." << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldApplyEqualsNotEqualsOperatorsTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class 1 should be instantiated in heap." << std::endl;

  wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p1{raw_ptr};
  // NOLINTNEXTLINE(performance-unnecessary-copy-initialization): Required
  wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p2{p1};
  wb::base::windows::com::com_ptr<IUnknown, &IID_IUnknown> pu{p1};

  EXPECT_TRUE(p1 == raw_ptr) << "Should equals with raw ptr." << std::endl;
  EXPECT_FALSE(p1 != raw_ptr) << "Should equals with raw ptr (2)." << std::endl;

  EXPECT_FALSE(p1 == nullptr) << "Should not equals nullptr." << std::endl;
  EXPECT_TRUE(p1 != nullptr) << "Should not equals nullptr (2)." << std::endl;

  EXPECT_TRUE(p1 == p1) << "Should equals with itself." << std::endl;
  EXPECT_FALSE(p1 != p1) << "Should equals with itself (2)." << std::endl;

  EXPECT_TRUE(p1 == p2) << "Should equals with self copy." << std::endl;
  EXPECT_FALSE(p1 != p2) << "Should equals with self copy (2)." << std::endl;

  EXPECT_TRUE(p1 == pu) << "Should equals with self copy as IUnknown."
                        << std::endl;
  EXPECT_FALSE(p1 != pu) << "Should equals with self copy as IUnknown (2)."
                         << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldApplyConstEqualsNotEqualsOperatorsTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class 1 should be instantiated in heap." << std::endl;

  const wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p1{
      raw_ptr};
  // NOLINTNEXTLINE(performance-unnecessary-copy-initialization): Required
  const wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p2{p1};
  // NOLINTNEXTLINE(performance-unnecessary-copy-initialization): Required
  const wb::base::windows::com::com_ptr<IUnknown, &IID_IUnknown> pu{p1};

  EXPECT_TRUE(p1 == raw_ptr) << "Should equals with raw ptr." << std::endl;
  EXPECT_FALSE(p1 != raw_ptr) << "Should equals with raw ptr (2)." << std::endl;

  EXPECT_FALSE(p1 == nullptr) << "Should not equals nullptr." << std::endl;
  EXPECT_TRUE(p1 != nullptr) << "Should not equals nullptr (2)." << std::endl;

  EXPECT_TRUE(p1 == p1) << "Should equals with itself." << std::endl;
  EXPECT_FALSE(p1 != p1) << "Should equals with itself (2)." << std::endl;

  EXPECT_TRUE(p1 == p2) << "Should equals with self copy." << std::endl;
  EXPECT_FALSE(p1 != p2) << "Should equals with self copy (2)." << std::endl;

  EXPECT_TRUE(p1 == pu) << "Should equals with self copy as IUnknown."
                        << std::endl;
  EXPECT_FALSE(p1 != pu) << "Should equals with self copy as IUnknown (2)."
                         << std::endl;
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldApplyAddressOfOperatorToEmptyComPtrTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class 1 should be instantiated in heap." << std::endl;
  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "Should set reference counter." << std::endl;

  wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p;

  IComptrTest **pp1 = &p;
  EXPECT_NE(pp1, nullptr) << "Should get interface pointer address."
                          << std::endl;

  *pp1 = raw_ptr;

  (*pp1)->AddRef();
  EXPECT_EQ(raw_ptr->counter, 2UL)
      << "Should update reference counter." << std::endl;
  EXPECT_EQ(p.GetInterfacePtr(), raw_ptr)
      << "Should set interface ptr." << std::endl;

  // Released through auto_deleter
  // (*pp1)->Release();

  // Pointer already set in com_ptr and will be released.
}

// NOLINTNEXTLINE(cert-err58-cpp)
GTEST_TEST(ComPtrTests, ShouldApplyAddressOfOperatorToExistingComPtrTest) {
  auto *raw_ptr = new (std::nothrow) ComptrTest;
  std::unique_ptr<ComptrTest> auto_deleter{raw_ptr};

  ASSERT_NE(raw_ptr, nullptr)
      << "COM class should be instantiated in heap." << std::endl;
  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "Should set reference counter." << std::endl;

  wb::base::windows::com::com_ptr<IComptrTest, &IID_ComptrTest> p{raw_ptr};
  EXPECT_EQ(raw_ptr->counter, 2UL)
      << "Should update reference counter." << std::endl;

  IComptrTest **pp1 = &p;
  EXPECT_NE(pp1, nullptr) << "Should get interface pointer address."
                          << std::endl;
  EXPECT_EQ(raw_ptr->counter, 1UL)
      << "Should release old reference." << std::endl;

  *pp1 = raw_ptr;

  (*pp1)->AddRef();
  EXPECT_EQ(raw_ptr->counter, 2UL)
      << "Should update reference counter." << std::endl;
  EXPECT_EQ(p.GetInterfacePtr(), raw_ptr)
      << "Should set interface ptr." << std::endl;

  // Released through auto_deleter
  // (*pp1)->Release();

  // Pointer already set in com_ptr and will be released.
}